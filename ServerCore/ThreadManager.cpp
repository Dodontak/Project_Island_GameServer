#include "pch.h"
#include "ThreadManager.h"
#include "GlobalQueue.h"
#include "JobQueue.h"

ThreadManager::ThreadManager() : _running(true)
{
	InitTLS();
}

ThreadManager::~ThreadManager()
{
	Join();
}

void ThreadManager::Launch(function<void()> callback)
{
	_workerThreads.push_back(thread([=]()
		{
			InitTLS();
			callback();
			DestroyTLS();
		}
	));
}

void ThreadManager::Join()
{
	for (auto& t : _workerThreads)
	{
		t.join();
	}
}

void ThreadManager::DoGlobalQueueWork()
{
	while (true)
	{
		uint64 now = ::GetTickCount64();
		if (now > LEndTickCount)
			break;

		JobQueueRef jobQueue = GGlobalQueue->Pop();
		if (jobQueue == nullptr)
			break;
		jobQueue->Execute();
	}
}

void ThreadManager::InitTLS()
{
	static atomic<int> SThreadId(1);
	LThreadId = SThreadId.fetch_add(1);
}

void ThreadManager::DestroyTLS()
{
}
