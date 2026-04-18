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
		service->GetIocpCore()->Dispatch(10);
	}
}

int main()
{
	cout << "=== Client ===" << endl;

	this_thread::sleep_for(chrono::seconds(1));
	ServerPacketHandler::Init();
	ThreadManager tManager;
	ClientServiceRef service = make_shared<ClientService>(
		NetAddress("127.0.0.1", 7777),
		[](ServiceRef service) {
			return make_shared<ServerSession>(service);
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
	this_thread::sleep_for(chrono::seconds(1));
	//while (true)
	//{
	//	this_thread::sleep_for(chrono::milliseconds(200));
	//	Protocol::C_CHAT pkt;
	//	pkt.set_msg("Hello Server!");
	//	service->broad_cast_test(ServerPacketHandler::MakeSendBuffer(pkt));
	//}
}