#include "pch.h"
#include "Service.h"
#include "Listener.h"

Service::Service(NetAddress listenerAddr) : _listenerAddr(listenerAddr)
{
	_iocpCore = make_shared<IocpCore>();
}

Service::~Service()
{
}

void Service::Start()
{
	SocketUtils::Init();

	ListenerRef listener = make_shared<Listener>(shared_from_this());

	listener->StartAccept();

	while (true)
	{
		_iocpCore->Dispatch();
	}
}
