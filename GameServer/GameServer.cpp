#include "pch.h"
#include "IocpCore.h"
#include "NetAddress.h"
#include "Listener.h"
#include "Service.h"
#include "ThreadManager.h"
#include "CoreTLS.h"

void WorkerThread(ServiceRef service)
{
	while (1)
	{
		service->GetIocpCore()->Dispatch();
		cout << "Worker thread " << LThreadId << " processed an I/O event" << endl;
	}
}

int main()
{
	ThreadManager threadManager;
	ServiceRef service = make_shared<Service>(NetAddress("0.0.0.0", 9000));
	service->Start();

	for (int i = 0; i < 5; i++)
	{
		threadManager.Launch([&service]() {
			WorkerThread(service);
			}
		);
	}

	this_thread::sleep_for(chrono::seconds(10));
	service->didconnect_all_test();
}
