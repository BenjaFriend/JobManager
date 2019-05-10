
#include "pch.h"
#include <iostream>
#include <vector>

#include "JobManager.h"

#define BUF_SIZE 4

struct TestDataBoi
{
    int hello = 0;
    int goodbye = 0;
    int iWonder = 0;
    int doYou = 0;
    uint64_t work2 = 0;
};

void empty_job(Job* aJob, const void* aData)
{
    const TestDataBoi* myArgs = static_cast<const TestDataBoi*>(aData);

    int sum = 0;
    for (int i = 0; i < 10000; ++i)
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

    for ( unsigned int j = 0; j < N; ++j )
    {
        empty_job( nullptr, nullptr );
    }

    // note when this was done
    end = std::chrono::steady_clock::now();

    auto singleThreadTime = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();

    std::cout << "Time to run (micro) = " << singleThreadTime << std::endl;
    
    // Run through the job system
    std::cout << "---- WITH JOB SYSTEM ----"<< std::endl;

    end = std::chrono::steady_clock::now();
    begin = std::chrono::steady_clock::now();

    TestDataBoi hey = { };
    hey.doYou = 1;
    hey.goodbye = -1;
    hey.iWonder = 12;
    hey.work2 = 69;

    Job* root = JobManager::Instance.CreateJob(&empty_job, &hey, sizeof(hey));

    for (unsigned int i = 0; i < N; ++i)
    {
        Job* child = JobManager::Instance.CreateJobAsChild(root, &empty_job, &hey, sizeof(hey));
        JobManager::Instance.Run(child);
    }

    JobManager::Instance.Run(root);
    JobManager::Instance.Wait(root);

    end = std::chrono::steady_clock::now();
    auto withJobSystemTime = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();

    std::cout << "Time to run (micro) = " << withJobSystemTime << std::endl;

    std::cout << "Difference (SingleThread - WithJobs) = " << (singleThreadTime - withJobSystemTime) << std::endl;

}


static void args_using_job(Job* aJob, const void* aData)
{

    // I wanna get a TestDataBoi pointer out of the aData


}

int main()
{
    //memory leak detections
#if defined( _DEBUG )
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
#endif

    // Create a job manager
    JobManager::Instance.Startup();

    TestPerfSimple();

    std::vector<TestDataBoi*> HeapData;

    for (int i = 0; i < BUF_SIZE; ++i)
    {
        TestDataBoi* ptr = new TestDataBoi();
        ptr->doYou = 1 * i;
        ptr->goodbye = 2 * i;
        ptr->hello = 3 * i;
        ptr->iWonder = 4 * i;
        ptr->work2 = (uint64_t)(5 * i);
        HeapData.emplace_back(ptr);
    }

    // Shutdown job manager
    JobManager::Instance.Shutdown();

    for (int i = 0; i < BUF_SIZE; ++i)
    {
        if (HeapData[i] != nullptr)
        {
            delete HeapData[i];
        }
    }

}
