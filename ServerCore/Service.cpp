#include "pch.h"
#include "Service.h"
#include "Listener.h"
#include "Session.h"

/*----------------------------------------------------------------------------*\
|                                  Service                                     |
\*----------------------------------------------------------------------------*/
Service::Service(NetAddress netAddr, SessionFactory sessionFactory) :
	_netAddress(netAddr), _sessionFactory(sessionFactory)
{
	_iocpCore = make_shared<IocpCore>();
}

Service::~Service() {}

SessionRef Service::CreateSession()
{
	SessionRef session = _sessionFactory(shared_from_this());
	session->_service = shared_from_this();

	return session;
}

SSL* Service::CreateSSL()
{
	return SSL_new(_ctx);
}

void Service::broad_cast_test(SendBufferRef sendBuffer)
{
	lock_guard<mutex> lock(_m);
	int n = 0;
	for (auto iocpObject : _sessions)
	{
		SessionRef session = static_pointer_cast<Session>(iocpObject);
		session->Send(sendBuffer);
		n++;
	}
	Utils::LockPrint("Send Data to ", n, " sessions");
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
ServerService::ServerService(NetAddress listenerAddr, SessionFactory sessionFactory,
	const char* certFile, const char* keyFile)
	: Service(listenerAddr, sessionFactory)
{
	const SSL_METHOD* method = TLS_server_method();
	_ctx = SSL_CTX_new(method);
	if (!_ctx)
		Utils::ErrorExit("Failed to create SSL_CTX in ServerService constructor");

	if (SSL_CTX_use_certificate_file(_ctx, certFile, SSL_FILETYPE_PEM) <= 0)
		Utils::ErrorExit("SSL_CTX_use_certificate_file error");

	if (SSL_CTX_use_PrivateKey_file(_ctx, keyFile, SSL_FILETYPE_PEM) <= 0)
		Utils::ErrorExit("SSL_CTX_use_PrivateKey_file error");
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
ClientService::ClientService(NetAddress serverAddr, SessionFactory sessionFactory, int32 clientCount)
	: Service(serverAddr, sessionFactory), _clientCount(clientCount)
{
	const SSL_METHOD* method = TLS_client_method();
	_ctx = SSL_CTX_new(method);
	if (!_ctx)
		Utils::ErrorExit("Failed to create SSL_CTX in ServerService constructor");
}

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