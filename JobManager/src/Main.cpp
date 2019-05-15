
#include "pch.h"
#include <iostream>
#include <vector>
#include <cstdint>
#include <inttypes.h>

#include "JobManager.h"

#define BUF_SIZE 4

struct TestData
{
	int hello = 0;
	int goodbye = 0;
	int iWonder = 0;
	int doYou = 0;
	uint64_t work2 = 0;
};

static void Printvalues( const TestData* aData )
{
	if( aData == nullptr )
	{
		return;
	}

	printf(
		"\n---\nHello: %d\n Goodbye: %d\n iWonder %d\n doYou %d\n work2: %" PRId64 "\n",
		aData->hello, aData->goodbye, aData->iWonder, aData->doYou, aData->work2
	);
}

void empty_job( Job* aJob, const void* aData )
{
	const TestData* myArgs = static_cast<const TestData*>( aData );
	if( myArgs )
	{
		Printvalues( myArgs );
	}
	int sum = 0;
	for( int i = 0; i < 10000; ++i )
	{
		sum += 1 * i;
	}
}

static void TestPerfSimple()
{
	std::cout << "==== TestPerfSimple ==== " << std::endl;

	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

	static const unsigned int N = 100000;

	std::cout << "---- Single thread ----" << std::endl;

	end = std::chrono::steady_clock::now();
	begin = std::chrono::steady_clock::now();

	for( unsigned int j = 0; j < N; ++j )
	{
		empty_job( nullptr, nullptr );
	}

	// note when this was done
	end = std::chrono::steady_clock::now();

	auto singleThreadTime = std::chrono::duration_cast<std::chrono::microseconds>( end - begin ).count();

	std::cout << "Time to run (micro) = " << singleThreadTime << std::endl;

	// Run through the job system
	std::cout << "---- WITH JOB SYSTEM ----" << std::endl;

	end = std::chrono::steady_clock::now();
	begin = std::chrono::steady_clock::now();

	TestData hey = {};
	hey.hello = 72;
	hey.goodbye = -1;
	hey.iWonder = 12;
	hey.doYou = 1;
	hey.work2 = 69;

	Job * root = JobManager::Instance.CreateJob( &empty_job, &hey, sizeof( hey ) );

	for( unsigned int i = 0; i < N; ++i )
	{
		Job* child = JobManager::Instance.CreateJobAsChild( root, &empty_job, &hey, sizeof( hey ) );
		JobManager::Instance.Run( child );
	}

	JobManager::Instance.Run( root );
	JobManager::Instance.Wait( root );

	end = std::chrono::steady_clock::now();
	auto withJobSystemTime = std::chrono::duration_cast<std::chrono::microseconds>( end - begin ).count();

	std::cout << "Time to run (micro) = " << withJobSystemTime << std::endl;

	std::cout << "Difference (SingleThread - WithJobs) = " << ( singleThreadTime - withJobSystemTime ) << std::endl;

}

static void args_using_job( Job * aJob, const void* aData )
{
	TestData* ptr = nullptr;
	memcpy( &ptr, aData, sizeof( TestData * * ) );

	if( ptr != nullptr )
	{
		Printvalues( ptr );
	}
}

void TestHeapArgs()
{
	std::vector<TestData*> HeapData;

	// Create some test data on the heap
	for( int i = 0; i < BUF_SIZE; ++i )
	{
		TestData* ptr = new TestData();
		ptr->doYou = 1 * ( i + 1 );
		ptr->goodbye = 2 * ( i + 1 );
		ptr->hello = 3 * ( i + 1 );
		ptr->iWonder = 4 * ( i + 1 );
		ptr->work2 = 67;
		HeapData.emplace_back( ptr );
	}

	TestData * *p = &HeapData[ 0 ];

	Job * root = JobManager::Instance.CreateJob( &args_using_job, &HeapData[ 0 ], sizeof( TestData * * ) );

	for( unsigned int i = 1; i < BUF_SIZE; ++i )
	{
		Job* child = JobManager::Instance.CreateJobAsChild( root, &args_using_job, &HeapData[ i ], sizeof( TestData * * ) );
		JobManager::Instance.Run( child );
	}

	JobManager::Instance.Run( root );
	JobManager::Instance.Wait( root );


	for( int i = 0; i < BUF_SIZE; ++i )
	{
		if( HeapData[ i ] != nullptr )
		{
			delete HeapData[ i ];
		}
	}
}

int main()
{
	//memory leak detections
#if defined( _DEBUG )
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	_CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_DEBUG );
#endif

	// Create a job manager
	JobManager::Instance.Startup();

	//TestPerfSimple();
	TestHeapArgs();

	// Shutdown job manager
	JobManager::Instance.Shutdown();



}
