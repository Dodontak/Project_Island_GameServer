#include "pch.h"
#include "Listener.h"
#include "IocpEvent.h"
#include "Session.h"
#include "Service.h"
#include <memory>

Listener::Listener(ServiceRef service) : _service(service), _address(service->GetAddr())
{
	_listenSocket = SocketUtils::CreateSocket();
	if (_listenSocket == INVALID_SOCKET)
		CRASH("Failed to create listen socket");
}

Listener::~Listener()
{
	SocketUtils::CloseSocket(_listenSocket);
}

void Listener::Dispatch(int32 numOfBytes, IocpEvent* event)
{
	if (event->GetEventType() != EventType::Accept)
		CRASH("Invalid event type for Listener");
	AcceptEvent* acceptEvent = static_cast<AcceptEvent*>(event);
	SessionRef session = acceptEvent->GetSession();

	ProcessAccept(session, acceptEvent);
	RegisterAccept(acceptEvent);
}

bool Listener::StartAccept()
{
	if (SocketUtils::SetReuseAddress(_listenSocket, true) == false)
		CRASH("Failed to set reuse listen socket");

	if (SocketUtils::SetTcpNoDelay(_listenSocket, true) == false)
		CRASH("Failed to set nodelay listen socket");

	if (SocketUtils::BindSocket(_listenSocket, _address) == false)
		CRASH("Failed to bind listen socket");

	if (SocketUtils::ListenSocket(_listenSocket, SOMAXCONN) == false)
		CRASH("Failed to listen on listen socket");

	if (_service->GetIocpCore()->RegisterHandle(shared_from_this()) == false)
		CRASH("Failed to register listen socket to IOCP");

	AcceptEvent* acceptEvent = new AcceptEvent();
	if (acceptEvent == nullptr)
		CRASH("Failed to create accept event");

	RegisterAccept(acceptEvent);

	return true;
}

void Listener::RegisterAccept(AcceptEvent* acceptEvent)
{
	acceptEvent->Init();
	acceptEvent->SetOwner(shared_from_this());
	SessionRef session = _service->CreateSession();
	if (session == nullptr)
		CRASH("Failed to create session for accept event");
	acceptEvent->SetSession(session);

	if (false == SocketUtils::AcceptEx(_listenSocket, session->_socket, acceptEvent->GetAcceptBuffer(), 0,
		sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, NULL, static_cast<LPOVERLAPPED>(acceptEvent)))
	{
		if (WSAGetLastError() != ERROR_IO_PENDING)
		{
			//TODO 적절한 처리
			RegisterAccept(acceptEvent);
		}
	}
}

void Listener::ProcessAccept(SessionRef session, AcceptEvent* acceptEvent)
{
	if (SocketUtils::SetUpdateAcceptSocket(session->_socket, _listenSocket) == false)
		return;

	if (session->SetAddressFromAcceptBuffer(acceptEvent->GetAcceptBuffer()) == false)
		return;

	if (_service->GetIocpCore()->RegisterHandle(session) == false)
		return;

	acceptEvent->Clear();

	session->ProcessConnect();
}



