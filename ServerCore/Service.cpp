#include "pch.h"
#include "Service.h"
#include "Listener.h"
#include "Session.h"

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
}

SessionRef Service::CreateSession()
{
	SOCKET clientSocket = SocketUtils::CreateSocket();
	if (clientSocket == INVALID_SOCKET)
		return nullptr;
	SessionRef session = make_shared<Session>(clientSocket);
	if (session == nullptr)
	{
		SocketUtils::CloseSocket(clientSocket);
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
