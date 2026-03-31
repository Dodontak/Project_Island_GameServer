#include "pch.h"
#include "Session.h"
#include "Service.h"
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
	ServiceRef service = make_shared<Service>(NetAddress("127.0.0.1", 7777));
	service->CreateSession();

	ThreadManager tManager;

	tManager.Launch([service]() {
		WorkerThread(service);
		}
	);
}