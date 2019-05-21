#pragma once

/************************************************************************/
/*         
 Followed this tutorial:
 https://blog.molecular-matters.com/2015/09/25/job-system-2-0-lock-free-work-stealing-part-3-going-lock-free/
 */
/************************************************************************/

#include "Job.h"

/// <summary>
/// A lock free work stealing queue
/// </summary>
class StealingQueue
{
public:
	
	/// <summary>
	/// Adds a job to the end of the queue
	/// </summary>
	/// <param name="aJob">Job to add to the queue</param>
	void Push( Job* aJob );

	/// <summary>
	/// Pops a job from the end of the queue
	/// </summary>
	/// <returns>Pointer to the next available job</returns>
	Job* Pop();

	/// <summary>
	/// Attempts to steal a job from the end of the queue
	/// </summary>
	/// <returns>Pointer to the next steal-able job</returns>
	Job* Steal();

private:
	/** Max number of jobs that this queue can allocate. Must be a power of 2 */
	static const unsigned int NUM_JOBS = 4096u;

	static const unsigned int MASK = NUM_JOBS - 1u;

	/** Memory block of possible jobs */
	Job* m_Jobs[ NUM_JOBS ];

	/** Marker for where the top of the queue is */
	volatile long m_Top = 0u;

	/** Marker for where the bottom of the queue is */
	volatile long m_Bottom = 0u;

};