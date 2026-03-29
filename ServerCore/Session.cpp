#include "pch.h"
#include "Session.h"
#include "IocpEvent.h"

Session::Session(SOCKET socket) : _socket(socket) {}

Session::~Session()
{
	if (_socket != INVALID_SOCKET)
		SocketUtils::CloseSocket(_socket);
}

void Session::Dispatch(int32 numOfBytes, IocpEvent* event)
{
	
	switch (event->GetEventType())
	{
	case EventType::Connect:
		break;
	case EventType::Disconnect:
		break;
	case EventType::Send:
		break;
	case EventType::Recv:
		ProcessRecv(numOfBytes, reinterpret_cast<RecvEvent*>(event));
		break;
	default:
		return;
	}
}

void Session::RegisterRecv(IocpEvent* recvEvent)
{
	WSABUF wsaBuf;
	wsaBuf.buf = reinterpret_cast<char*>(_recvBuffer);
	wsaBuf.len = BUFFER_SIZE;

	DWORD numOfBytes = 0;
	DWORD flags = 0;

	if (SOCKET_ERROR == WSARecv(_socket, &wsaBuf, 1, OUT & numOfBytes, &flags,
		static_cast<OVERLAPPED*>(recvEvent), nullptr))
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			//TODO 적절한 처리
		}
	}
}

void Session::ProcessRecv(int32 numOfBytes, RecvEvent* recvEvent)
{
	cout << "Received " << numOfBytes << " bytes from client" << endl;
	::memcpy(_sendBuffer, _recvBuffer, numOfBytes);
	_sendBuffer[numOfBytes] = '\0';
	cout << _sendBuffer << endl;
	RegisterRecv(recvEvent);
}

bool Session::SetAddressFromAcceptBuffer(BYTE* buffer)
{
	SOCKADDR_IN* serverAddr = nullptr;
	SOCKADDR_IN* clientAddr = nullptr;
	int32 serverAddrLen = 0;
	int32 clientAddrLen = 0;
	SocketUtils::GetAcceptExSockaddrs(
		buffer,
		0,
		sizeof(SOCKADDR_IN) + 16,
		sizeof(SOCKADDR_IN) + 16,
		(SOCKADDR**)&serverAddr, &serverAddrLen,
		(SOCKADDR**)&clientAddr, &clientAddrLen
	);
	if (serverAddr == nullptr || clientAddr == nullptr)
		return false;
	_address.SetAddr(*clientAddr);
	return true;
}
