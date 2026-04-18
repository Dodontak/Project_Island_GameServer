#pragma once

#include "LockQueue.h"

class GlobalQueue
{
public:
	GlobalQueue() {}
	~GlobalQueue() {}

	void Push(JobQueueRef jobQueue) { _jobQueues.Push(jobQueue); }
	JobQueueRef Pop() { return _jobQueues.Pop(); }

private:
	LockQueue<JobQueueRef> _jobQueues;
};