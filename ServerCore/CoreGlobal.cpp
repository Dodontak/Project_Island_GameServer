#include "pch.h"
#include "CoreGlobal.h"
#include "DBConnectionPool.h"
#include "GlobalQueue.h"
#include "JobTimer.h"

DBConnectionPool* GDBConnectionPool = nullptr;
GlobalQueue* GGlobalQueue = nullptr;
JobTimer* GJobTimer = nullptr;

class CoreGlobal
{
public:
	CoreGlobal()
	{
		GDBConnectionPool = new DBConnectionPool();
		GGlobalQueue = new GlobalQueue();
		GJobTimer = new JobTimer();
	}
	~CoreGlobal()
	{
		delete GDBConnectionPool;
		delete GGlobalQueue;
		delete GJobTimer;
	}
}	GcoreGlobal;