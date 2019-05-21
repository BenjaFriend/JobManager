#include "pch.h"
#include "JobManager.h"

thread_local Job JobManager::g_jobAllocator[ MAX_JOB_COUNT ];
thread_local uint32_t JobManager::g_allocatedJobs = 0u;

JobManager JobManager::Instance = {};
std::vector<std::thread> JobManager::g_WorkerThreads;
std::unordered_map<std::thread::id, StealingQueue> JobManager::g_JobQueues;

void JobManager::Startup()
{
	IsDone = false;

	// Create a thread pool to the worker thread
	const unsigned int threadCount = GetAmountOfSupportedThreads();

	for( size_t i = 0; i < threadCount; ++i )
	{
		g_WorkerThreads.push_back( std::thread( &JobManager::WorkerThread, this ) );
		std::thread::id id = g_WorkerThreads[ i ].get_id();
		g_JobQueues[ id ] = StealingQueue();
	}
}

void JobManager::Shutdown()
{
	IsDone = true;

	for( auto& item : g_WorkerThreads )
	{
		item.join();
	}
}

Job* JobManager::CreateJob( JobFunction aFunction, void* args, size_t aSize )
{
	assert( aSize >= 0 && aSize < JOB_DATA_PADDING_SIZE );

	Job * job = AllocateJob();
	job->Function = aFunction;
	job->Parent = nullptr;
	memset( job->Padding, '\0', JOB_DATA_PADDING_SIZE );

	// Memcpy the args to the jobs padding
	if( args != nullptr )
	{
		memcpy( job->Padding, args, aSize );
	}

	job->UnfinishedJobs.store( 1 );

	return job;
}

Job* JobManager::CreateJobAsChild( Job* aParent, JobFunction aFunction, void* args, size_t aSize )
{
	assert( aSize >= 0 && aSize < JOB_DATA_PADDING_SIZE );

	// Keep track of the number of jobs on the parent
	aParent->UnfinishedJobs.fetch_add( 1 );

	// Create a new job
	Job * job = AllocateJob();
	job->Function = aFunction;
	job->Parent = aParent;
	memset( job->Padding, '\0', JOB_DATA_PADDING_SIZE );

	// Memcpy the args to the jobs padding
	if( args != nullptr )
	{
		memcpy( job->Padding, args, aSize );
	}

	job->UnfinishedJobs.store( 1 );

	return job;
}

void JobManager::Run( Job* aJob )
{
	std::thread::id id = ::std::this_thread::get_id();
	g_JobQueues[ id ].Push( aJob );
}

void JobManager::Wait( const Job* aJob )
{
	// Wait until this job has completed
	// in the meantime, work on another job
	while( !HasJobCompleted( aJob ) )
	{
		Job* nextJob = GetJob();
		if( nextJob )
		{
			Execute( nextJob );
		}
	}
}

const unsigned int JobManager::GetAmountOfSupportedThreads()
{
	return std::thread::hardware_concurrency();
}

void JobManager::WorkerThread()
{
	while( !IsDone )
	{
		Job* job = GetJob();
		if( job )
		{
			Execute( job );
		}
	}
}

void JobManager::YieldWorker()
{
	// For now, simply sleep this thread to yield its time
	std::this_thread::sleep_for( std::chrono::milliseconds( 3 ) );
}

Job* JobManager::GetJob()
{
	std::thread::id id = ::std::this_thread::get_id();
	StealingQueue * q = &g_JobQueues[ id ];

	Job* CurJob = q->Pop();

	if( CurJob )
	{
		return CurJob;
	}
	else
	{
		auto randQueue = g_JobQueues.begin();
		// #TODO: Make this get a random queue
		std::advance( randQueue, Utils::Random::Random0ToN( g_JobQueues.size() ) );

		StealingQueue* stealQueue = &randQueue->second;
		// Make sure that we don't steal from ourselves
		if( stealQueue == q )
		{
			YieldWorker();
			return nullptr;
		}
		
		// try and steal a job from another job queue
		Job* stolenJob = stealQueue->Steal();
		if( stolenJob )
		{
			return stolenJob;
		}

		// We couldn't find out job, 
		YieldWorker();
		return nullptr;
	}

	return nullptr;
}

bool JobManager::HasJobCompleted( const Job* aJob )
{
	// A job is done if there is no more unfinished work
	return ( aJob->UnfinishedJobs <= 0 );
}

void JobManager::Execute( Job * aJob )
{
	assert( aJob != nullptr );
	// Call the jobs function
	( aJob->Function )( aJob, aJob->Padding );
	// Finish the job when it is done
	Finish( aJob );
}

Job* JobManager::AllocateJob()
{
	const uint32_t index = g_allocatedJobs++;
	return &g_jobAllocator[ index & ( MAX_JOB_COUNT - 1u ) ];
}

void JobManager::Finish( Job * aJob )
{
	// Decrement the jobs unfinished job count
	const int32_t unfinishedJobs = --aJob->UnfinishedJobs;

	// If this job is done, then let the jobs parent know
	if( ( unfinishedJobs == 0 ) && ( aJob->Parent ) )
	{
		Finish( aJob->Parent );
	}
}