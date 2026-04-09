#include "pch.h"
#include "CoreGlobal.h"
#include "DBConnectionPool.h"

DBConnectionPool* GDBConnectionPool = nullptr;

class CoreGlobal
{
public:
	CoreGlobal()
	{
		GDBConnectionPool = new DBConnectionPool();
	}
	~CoreGlobal()
	{
		delete GDBConnectionPool;
	}
}	GcoreGlobal;