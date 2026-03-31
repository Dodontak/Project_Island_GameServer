#include "pch.h"
#include "Service.h"
#include "Listener.h"
#include "Session.h"

/*----------------------------------------------------------------------------*\
|                                  Service                                     |
\*----------------------------------------------------------------------------*/

Service::Service(NetAddress netAddr) : _netAddress(netAddr)
{
	_iocpCore = make_shared<IocpCore>();
}

Service::~Service()
{
}

SessionRef Service::CreateSession()
{
	SOCKET socket = SocketUtils::CreateSocket();
	if (socket == INVALID_SOCKET)
		return nullptr;
	SessionRef session = make_shared<Session>(socket);
	if (session == nullptr)
	{
		SocketUtils::CloseSocket(socket);
		return nullptr;
	}
	session->_service = shared_from_this();

	return session;
}

void Service::broad_cast_test(SendBufferRef sendBuffer)
{
	lock_guard<mutex> lock(_m);
	for (auto iocpObject : _sessions)
	{
		SessionRef session = static_pointer_cast<Session>(iocpObject);
		session->Send(sendBuffer);
	}
}

void Service::didconnect_all_test()
{
	lock_guard<mutex> lock(_m);
	for (auto it = _sessions.begin(); it != _sessions.end();)
	{
		(*it)->RegisterDisconnect();
		it = _sessions.erase(it);
	}
}

void Service::AddSession(SessionRef session)
{
	lock_guard<mutex> lock(_m);
	_sessions.insert(session);
}

void Service::RemoveSession(SessionRef session)
{
	lock_guard<mutex> lock(_m);
	_sessions.erase(session);
}

/*----------------------------------------------------------------------------*\
|                               ServerService                                  |
\*----------------------------------------------------------------------------*/
ServerService::ServerService(NetAddress listenerAddr) : Service(listenerAddr)
{
}

void ServerService::Start()
{
	SocketUtils::Init();

	ListenerRef listener = make_shared<Listener>(shared_from_this());

	listener->StartAccept();
}



/*----------------------------------------------------------------------------*\
|                               ClientService                                  |
\*----------------------------------------------------------------------------*/
ClientService::ClientService(NetAddress serverAddr, int32 clientCount)
	: Service(serverAddr), _clientCount(clientCount) {}

void ClientService::Start()
{
	SocketUtils::Init();

	for (int32 i = 0; i < _clientCount; i++)
	{
		SessionRef session = CreateSession();

		_iocpCore->RegisterHandle(session);

		session->SetAddr(_netAddress);

		session->RegisterConnect();
	}
}