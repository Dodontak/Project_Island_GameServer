#include "pch.h"
#include "CoreGlobal.h"
#include "DBConnectionPool.h"
#include "GlobalQueue.h"

DBConnectionPool* GDBConnectionPool = nullptr;
GlobalQueue* GGlobalQueue = nullptr;

class CoreGlobal
{
public:
	CoreGlobal()
	{
		GDBConnectionPool = new DBConnectionPool();
		GGlobalQueue = new GlobalQueue();
	}
	~CoreGlobal()
	{
		delete GDBConnectionPool;
		delete GGlobalQueue;
	}
}	GcoreGlobal;