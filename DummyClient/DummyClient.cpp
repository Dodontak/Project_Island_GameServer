#include "pch.h"
#include "Session.h"
#include "Service.h"
#include "SendBuffer.h"
#include "ThreadManager.h"

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
	ThreadManager tManager;
	ClientServiceRef service = make_shared<ClientService>(
		NetAddress("127.0.0.1", 7777),
		2
	);

	service->Start();

	for (int i = 0; i < 5; i++)
	{
		tManager.Launch([service]() {
			WorkerThread(service);
			}
		);
	}
	//while (true)
	//{
		this_thread::sleep_for(chrono::seconds(1));
		string msg = "Hello Iocp Server!";

		SendBufferRef sendBuffer = make_shared<SendBuffer>((BYTE*)msg.c_str(), msg.length());
		service->broad_cast_test(sendBuffer);
	//}
}