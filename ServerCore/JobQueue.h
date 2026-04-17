#pragma once
#include "Job.h"
#include "LockQueue.h"

class JobQueue : public enable_shared_from_this<JobQueue>
{
public:
	void DoAsync(function<void()>&& callback)
	{
		Push(make_shared<Job>(std::move(callback)));
	}

	template<typename T, typename Ret, typename... Args>
	void DoAsync(Ret(T::* memFunc)(Args...), Args... args)
	{
		shared_ptr<T> owner = static_pointer_cast<T>(shared_from_this());
		Push(make_shared<Job>(owner, memFunc, std::forward<Args>(args)...));
	}

	void					ClearJobs() { _jobs.Clear(); }

public:
	void					Push(JobRef job, bool pushOnly = false);
	void					Execute();

protected:
	LockQueue<JobRef>		_jobs;
	atomic<int32>			_jobCount = 0;
};

