#include "pch.h"
#include "Session.h"
#include "Service.h"
#include "SendBuffer.h"
#include "ThreadManager.h"
#include "ServerSession.h"
#include "ServerPacketHandler.h"

void WorkerThread(ServiceRef service)
{
	while (1)
	{
		service->GetIocpCore()->Dispatch();
	}
}

int main()
{
	cout << "=== DummyClient ===" << endl;

	this_thread::sleep_for(chrono::seconds(1));
	ServerPacketHandler::Init();
	ThreadManager tManager;
	ClientServiceRef service = make_shared<ClientService>(
		NetAddress("127.0.0.1", 7777),
		[](SOCKET socket) {
			return make_shared<ServerSession>(socket);
		},
		3
	);

	service->Start();

	for (int i = 0; i < 5; i++)
	{
		tManager.Launch([service]() {
			WorkerThread(service);
			}
		);
	}
	while (true)
	{
		this_thread::sleep_for(chrono::seconds(1));
		Protocol::C_CHAT pkt;
		pkt.set_msg("Hello Iocp Server!");
		service->broad_cast_test(ServerPacketHandler::MakeSendBuffer(pkt));
	}
}