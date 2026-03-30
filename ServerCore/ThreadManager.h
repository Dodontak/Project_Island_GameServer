#pragma once

#include <functional>
#include <thread>

class ThreadManager
{
public:
	ThreadManager();
	~ThreadManager();

	void Launch(function<void()> callback);
	void Join();

	bool IsRunning() { return _running; }

private:
	static void InitTLS();
	static void DestroyTLS();
	bool _running;

	vector<thread> _workerThreads;
};

