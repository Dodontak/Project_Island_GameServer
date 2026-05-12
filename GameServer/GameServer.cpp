#include "pch.h"
#include "IocpCore.h"
#include "NetAddress.h"
#include "Listener.h"
#include "Service.h"
#include "ThreadManager.h"
#include "ClientPacketHandler.h"
#include "GameSession.h"
#include "DBConnectionPool.h"

#include "Room.h"

enum
{ //64ms넘어가면 다른 스레드에 일감을 넘김.
	WORKER_TICK = 64
};

void WorkerThread(ServiceRef service)
{
	while (true)
	{
		LEndTickCount = ::GetTickCount64() + WORKER_TICK;
		service->GetIocpCore()->Dispatch(10);

		ThreadManager::DoGlobalQueueWork();

		ThreadManager::DistributeReservedJobs();
	}
}

int main()
{
	cout << "=== Server ===" << endl;
	//Room 테스트용
	GRoom[0] = make_shared<Room>();
	GRoom[1] = make_shared<Room>();
	ClientPacketHandler::Init();
	//GDBConnectionPool->Init(10, "192.168.0.39", 6379, 10,
	//	"host=192.168.0.39 user=postgres port=5432 "
	//	"dbname=postgres password=password "
	//	"connect_timeout=3");
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

	for (int i = 0; i < 10; i++)
	{
		threadManager.Launch([&service]() {
			WorkerThread(service);
			}
		);
	}
	//while (true)
	//{
	//	Protocol::GS_CHAT pkt;
	//	pkt.set_msg("HelloWorld!");
	//	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(pkt);

	//	service->broad_cast_test(sendBuffer);
	//	this_thread::sleep_for(1s);
	//}
}
