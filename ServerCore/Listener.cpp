#include "pch.h"
#include "Listener.h"
#include "IocpEvent.h"
#include "Session.h"
#include "Service.h"
#include <memory>

Listener::Listener(ServiceRef service) : _service(service), _address(service->GetListenerAddr())
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

	ProcessAccept(session);
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

	if (_service->GetIocpCore()->RegisterHandle(GetHandle()) == false)
		CRASH("Failed to register listen socket to IOCP");

	AcceptEvent* acceptEvent = new AcceptEvent(shared_from_this());
	if (acceptEvent == nullptr)
		CRASH("Failed to create accept event");

	RegisterAccept(acceptEvent);

	return true;
}

void Listener::RegisterAccept(AcceptEvent* acceptEvent)
{
	SOCKET clientSocket = SocketUtils::CreateSocket();
	if (clientSocket == INVALID_SOCKET)
		return;

	SessionRef session = make_shared<Session>(clientSocket);
	if (acceptEvent == nullptr)
		CRASH("Failed to create accept event");
	acceptEvent->SetSession(session);

	if (false == SocketUtils::AcceptEx(_listenSocket, clientSocket, session->GetRecvBuffer(), 0,
		sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, NULL, static_cast<LPOVERLAPPED>(acceptEvent)))
	{
		if (WSAGetLastError() != ERROR_IO_PENDING)
		{
			//TODO 적절한 처리
		}
	}
}

void Listener::ProcessAccept(SessionRef session)
{
	if (SocketUtils::SetUpdateAcceptSocket(session->_socket, _listenSocket) == false)
		return;
	
	if (session->SetAddressFromAcceptBuffer() == false)
		return;

	if (_service->GetIocpCore()->RegisterHandle(session->GetHandle()) == false)
		return;

	RecvEvent* recvEvent = new RecvEvent(session);
	if (recvEvent == nullptr)
		return;

	session->RegisterRecv(recvEvent);
}



