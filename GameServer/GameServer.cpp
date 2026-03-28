#include "pch.h"
#include "IocpCore.h"
#include "NetAddress.h"
#include "Listener.h"
#include "Service.h"

int main()
{
	ServiceRef service = make_shared<Service>(NetAddress("0.0.0.0", 9000));
	service->Start();
}
