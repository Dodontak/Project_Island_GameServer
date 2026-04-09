#include "pch.h"
#include "IocpCore.h"
#include "NetAddress.h"
#include "Listener.h"
#include "Service.h"
#include "ThreadManager.h"
#include "CoreTLS.h"
#include "ClientPacketHandler.h"
#include "GameSession.h"
#include "DBConnectionPool.h"

void WorkerThread(ServiceRef service)
{
	while (1)
	{
		service->GetIocpCore()->Dispatch();
	}
}

int main()
{
	cout << "=== Server ===" << endl;
	ClientPacketHandler::Init();
	GDBConnectionPool->Init(10, "192.168.0.38", 6379, 10,
		"host=192.168.0.38 user=postgres port=5432 "
		"dbname=postgres password=password "
		"connect_timeout=3");
	ThreadManager threadManager;
	ServiceRef service = make_shared<ServerService>(
		NetAddress("0.0.0.0", 7777),
		[](ServiceRef service) {
			return make_shared<GameSession>(service);
		},
		R"(SSL\server.crt)",
		R"(SSL\server.key)"
	);
	service->Start();

	for (int i = 0; i < 5; i++)
	{
		threadManager.Launch([&service]() {
			WorkerThread(service);
			}
		);
	}
}
