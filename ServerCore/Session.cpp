#include "pch.h"
#include "Session.h"
#include "IocpEvent.h"
#include "Service.h"
#include "SendBuffer.h"

Session::Session(SOCKET socket) : _socket(socket), _recvBuffer(BUFFER_SIZE) {}

Session::~Session()
{
	cout << "Session " << _socket << " distructed" << endl;
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
		ProcessSend(numOfBytes);
		break;
	case EventType::Recv:
		ProcessRecv(numOfBytes);
		break;
	default:
		return;
	}
}

void Session::RegisterRecv()
{
	_recvEvent.Init();
	_recvEvent.SetOwner(shared_from_this());

	WSABUF wsaBuf;
	wsaBuf.buf = reinterpret_cast<char*>(_recvBuffer.WritePos());
	wsaBuf.len = _recvBuffer.FreeSize();

	DWORD numOfBytes = 0;
	DWORD flags = 0;

	if (SOCKET_ERROR == WSARecv(_socket, &wsaBuf, 1, OUT & numOfBytes, &flags, static_cast<OVERLAPPED*>(&_recvEvent), nullptr))
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			//TODO 적절한 처리
		}
	}
}

void Session::ProcessRecv(int32 numOfBytes)
{
	_recvEvent.Clear();
	if (numOfBytes == 0) // 클라이언트가 정상적으로 연결을 종료한 경우
	{
		ProcessDisconnect();
		return;
	}
	_recvBuffer.OnWrite(numOfBytes);
	SendBufferRef sendBuffer = make_shared<SendBuffer>(_recvBuffer.ReadPos(), numOfBytes);
	_recvBuffer.OnRead(numOfBytes);
	_recvBuffer.Clean();
	if (ServiceRef service = _service.lock())
	{
		service->broad_cast_test(sendBuffer);
	}

	RegisterRecv();
}

void Session::RegisterSend()
{
	_sendEvent.Init();
	_sendEvent.SetOwner(shared_from_this());

	while (!_sendBuffers.empty())
	{
		SendBufferRef sendBuffer = _sendBuffers.front();
		_sendBuffers.pop();
		_sendEvent.Push(sendBuffer);
	}

	vector<WSABUF> wsaBufs;
	for (auto sendBuffer : _sendEvent._sendBuffers)
	{
		WSABUF wsaBuf;
		wsaBuf.buf = reinterpret_cast<char*>(sendBuffer->GetBuffer());
		wsaBuf.len = sendBuffer->GetDataLen();
		wsaBufs.push_back(wsaBuf);
	}
	DWORD numOfBytes = 0;
	if (SOCKET_ERROR == WSASend(_socket, wsaBufs.data(), static_cast<DWORD>(wsaBufs.size()),
		OUT & numOfBytes, 0, (LPWSAOVERLAPPED)&_sendEvent, nullptr))
	{
		int errorCode = WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			//TODO 적절한 처리
		}
	}
}

void Session::ProcessSend(int32 numOfBytes)
{
	_sendEvent.Clear();

}

void Session::RegisterDisconnect()
{

}

void Session::ProcessDisconnect()
{
	if (ServiceRef service = _service.lock())
	{
		service->RemoveSession(static_pointer_cast<Session>(shared_from_this()));
		service->broad_cast_test(make_shared<SendBuffer>((BYTE*)"A client has disconnected\n", 28));
	}
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
