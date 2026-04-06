#include "pch.h"
#include "IocpCore.h"
#include "NetAddress.h"
#include "Listener.h"
#include "Service.h"
#include "ThreadManager.h"
#include "CoreTLS.h"
#include "ClientPacketHandler.h"
#include "GameSession.h"

void WorkerThread(ServiceRef service)
{
	while (1)
	{
		service->GetIocpCore()->Dispatch();
	}
}

int main()
{
	cout << "=== GameServer ===" << endl;
	ClientPacketHandler::Init();
	ThreadManager threadManager;
	ServiceRef service = make_shared<ServerService>(
		NetAddress("0.0.0.0", 7777),
		[](SOCKET socket) {
			return make_shared<GameSession>(socket);
		},
		R"(SSL\server.crt)",
		R"(SSL\server.key)"
	);
	service->Start();

	for (int i = 0; i < 10; i++)
	{
		threadManager.Launch([&service]() {
			WorkerThread(service);
			}
		);
	}
}
