#include "pch.h"
#include "NetAddress.h"

int main()
{
	SocketUtils::Init();

	NetAddress addr("127.0.0.1", 7777);
	cout << addr.GetIp() << ":" << addr.GetPort() << endl;
}
