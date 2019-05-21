#include "pch.h"
#include "StealingQueue.h"

void StealingQueue::Push( Job* aJob )
{
	long b = m_Bottom;
	m_Jobs[ b & MASK ] = aJob;

	// ensure the job is written before b+1 is published to other threads.
	// on x86/64, a compiler barrier is enough.
	COMPILER_BARRIER;

	m_Bottom = b + 1;
}

Job* StealingQueue::Pop()
{
	long b = m_Bottom - 1;
	m_Bottom = b;

	_InterlockedExchange( &m_Bottom, b );

	long t = m_Top;
	if( t <= b )
	{
		Job* job = m_Jobs[ b & MASK ];

		if( t != b )
		{
			return job;
		}

		// __sync_bool_compare_and_swap on gcc
		// this is the last item in the queue
		if( _InterlockedCompareExchange( &m_Top, t + 1, t ) != t )
		{
			// failed race against steal operation
			job = nullptr;
		}

		m_Bottom = t + 1;
		return job;
	}
	else
	{
		m_Bottom = t;
		return nullptr;
	}
}

Job* StealingQueue::Steal()
{
	// Lock ----
	long t = m_Top;

	// ensure that top is always read before bottom.
	// loads will not be reordered with other loads on x86, so a compiler barrier is enough.
	COMPILER_BARRIER;

	long b = m_Bottom;

	if( t < b )
	{
		Job* job = m_Jobs[ t & MASK ];

		if( _InterlockedCompareExchange( &m_Top, t + 1, t ) != t )
		{
			// a concurrent steal or pop operation removed an element from the deque in the meantime.
			return nullptr;
		}

		return job;
	}
	else
	{
		// Empty queue
		return nullptr;
	}

}
