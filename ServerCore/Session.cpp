#include "pch.h"
#include "Session.h"
#include "IocpEvent.h"
#include "Service.h"
#include "SendBuffer.h"

Session::Session(ServiceRef service) : _service(service), _recvBuffer(BUFFER_SIZE)
{
	_socket = SocketUtils::CreateSocket();
	if (_socket == INVALID_SOCKET)
		Utils::HandleError("Failed to create socket in Session constructor");
	Utils::LockPrint("Session ", _socket, " constructed");
}

Session::~Session()
{
	Utils::LockPrint("Session ", _socket, " distructed");
	if (_socket != INVALID_SOCKET)
		SocketUtils::CloseSocket(_socket);
}

void Session::Dispatch(int32 numOfBytes, IocpEvent* event)
{
	switch (event->GetEventType())
	{
	case EventType::Connect:
		ProcessConnect();
		break;
	case EventType::Disconnect:
		ProcessDisconnect();
		break;
	case EventType::Send:
		ProcessSend(numOfBytes);
		break;
	case EventType::Recv:
		ProcessRecv(numOfBytes);
		break;
	case EventType::TLSHandshakeAcceptRecv:
		ProcessTLSHandshakeAcceptRecv(numOfBytes);
		break;
	case EventType::TLSHandshakeConnectRecv:
		ProcessTLSHandshakeConnectRecv(numOfBytes);
		break;
	default:
		return;
	}
}

/*----------------------------------------------------------------------------*\
|                                  Recv                                        |
\*----------------------------------------------------------------------------*/
void Session::RegisterRecv()
{
	if (_isConnected == false)
		return;
	_recvEvent.Init();
	_recvEvent.SetOwner(shared_from_this());

	WSABUF wsaBuf;
	wsaBuf.buf = reinterpret_cast<char*>(GetEncRecvBuffer().WritePos());
	wsaBuf.len = GetEncRecvBuffer().FreeSize();

	DWORD numOfBytes = 0;
	DWORD flags = 0;

	if (SOCKET_ERROR == WSARecv(_socket, &wsaBuf, 1, OUT & numOfBytes, &flags, static_cast<OVERLAPPED*>(&_recvEvent), nullptr))
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			//TODO 적절한 처리
			_recvEvent.Clear();
		}
	}
}

void Session::ProcessRecv(int32 numOfBytes)
{
	_recvEvent.Clear();
	if (numOfBytes == 0) // 클라이언트가 정상적으로 연결을 종료한 경우
	{
		RegisterDisconnect();
		return;
	}

	RecvBuffer& encBuffer = GetEncRecvBuffer();
	RecvBuffer& decBuffer = GetDecRecvBuffer();

	encBuffer.OnWrite(numOfBytes);

	// enc의 데이터를 복호화 해서 dec로 이동
	// encBuffer.OnRead, decBuffer.OnWrite는 내부에서 호출해줌
	do {
		uint8 ret = Decrypt(encBuffer, decBuffer);
		switch (ret)
		{
		case 0: // 성공
			break;
		case 1: // 상대가 shutdown. shutdown 호출 가능
			//TODO shutdown 정상종료
			break;
		case 2: // 에러. shutdown 호출 불가능.
			RegisterDisconnect();
			break;
		}
	} while (HasSslPendingData());

	int processLen = OnRecv(decBuffer.ReadPos(), decBuffer.DataSize());
	decBuffer.OnRead(processLen);

	encBuffer.Clean();
	decBuffer.Clean();

	RegisterRecv();
}

/*----------------------------------------------------------------------------*\
|                                  Send                                        |
\*----------------------------------------------------------------------------*/
void	 Session::Send(SendBufferRef sendBuffer)
{
	SendBufferRef encBuffer;
	bool isSuccess = Encrypt(sendBuffer, encBuffer);
	{
		lock_guard<mutex> lock(_m);
		_sendBuffers.push(encBuffer);
	}

	bool expected = false;
	if (_sendRegistered.compare_exchange_strong(expected, true))
	{
		RegisterSend();
	}
}

void Session::RegisterSend()
{
	if (_isConnected == false)
		return;
	_sendEvent.Init();
	_sendEvent.SetOwner(shared_from_this());
	{
		lock_guard<mutex> lock(_m);
		while (!_sendBuffers.empty())
		{
			SendBufferRef sendBuffer = _sendBuffers.front();
			_sendBuffers.pop();
			_sendEvent.PushBack(sendBuffer);
		}
	}

	vector<WSABUF> wsaBufs;
	for (auto& sendBuffer : _sendEvent.GetSendBuffers())
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
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			//TODO 적절한 처리
			_sendEvent.Clear();
		}
	}
}

void Session::ProcessSend(int32 numOfBytes)
{
	//Send 0
	if (numOfBytes == 0)
	{
		_sendEvent.Clear();
		RegisterDisconnect();
		return;
	}
	// 보낸 바이트 수 < 보내려 했던 바이트 일 경우 처리.
	if (numOfBytes < _sendEvent.GetWantSendBytes())
	{
		uint32 sendedBytes = numOfBytes;
		deque<SendBufferRef>& sendBuffers = _sendEvent.GetSendBuffers();
		while (!sendBuffers.empty())
		{
			if (sendedBytes >= sendBuffers.front()->GetDataLen())
			{
				sendedBytes -= sendBuffers.front()->GetDataLen();
				sendBuffers.pop_front();
			}
			else
			{
				BYTE* pos = sendBuffers.front()->GetPosPtr(sendedBytes);
				int32 dataLen = sendBuffers.front()->GetDataLen() - sendedBytes;

				SendBufferRef newone = make_shared<SendBuffer>(pos, dataLen);
				sendBuffers.pop_front();
				_sendEvent.PushFront(newone);
				RegisterSend();
				return;
			}
		}
		return;
	}

	_sendEvent.Clear();
	{
		lock_guard<mutex> lock(_m);
		if (_sendBuffers.empty())
		{
			_sendRegistered.store(false);
			return;
		}
	}
	RegisterSend();
}

/*----------------------------------------------------------------------------*\
|                                  Connect                                     |
\*----------------------------------------------------------------------------*/
void Session::RegisterConnect()
{
	_connectEvent.Init();
	_connectEvent.SetOwner(shared_from_this());

	if (SocketUtils::BindSocket(_socket, NetAddress()) == false)
	{
		cerr << "Failed to bind session socket in Session::RegisterConnect()" << endl;
		ProcessDisconnect();
		return;
	}

	DWORD BytesSent = 0;
	if (false == SocketUtils::ConnectEx(_socket, (const sockaddr*)&_address.GetAddr(),
		sizeof(sockaddr), NULL, 0, &BytesSent, &_connectEvent))
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			//TODO 적절한 처리
			_connectEvent.Clear();
		}
	}
}

void Session::ProcessConnect()
{
	_connectEvent.Clear();

	_isConnected.store(true);
	if (ServiceRef service = _service.lock())
		service->AddSession(static_pointer_cast<Session>(shared_from_this()));

	TLSConnect();
}

/*----------------------------------------------------------------------------*\
|                                  Disconnect                                  |
\*----------------------------------------------------------------------------*/
void Session::RegisterDisconnect()
{
	bool expected = true;
	if (false == _isConnected.compare_exchange_strong(expected, false))
		return;
	_disconnectEvent.Init();
	_disconnectEvent.SetOwner(shared_from_this());

	if (false == SocketUtils::DisconnectEx(_socket, &_disconnectEvent, 0, 0))
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			//TODO 적절한 처리
			_disconnectEvent.Clear();
		}
	}
}

void Session::ProcessDisconnect()
{
	_disconnectEvent.Clear();
	_isConnected.store(false);
	if (ServiceRef service = _service.lock())
	{
		service->RemoveSession(static_pointer_cast<Session>(shared_from_this()));
		service->broad_cast_test(make_shared<SendBuffer>((BYTE*)"A client has disconnected", 27));
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

void Session::ProcessTLSHandshakeAcceptRecv(int32 numOfBytes)
{
	CRASH("Session::ProcessTLS called in NOT TLS Session");
}

void Session::ProcessTLSHandshakeConnectRecv(int32 numOfBytes)
{
	CRASH("Session::ProcessTLS called in NOT TLS Session");
}

void Session::TLSAccept()
{
	RegisterRecv();
}

void Session::TLSConnect()
{
	RegisterRecv();
}

bool Session::Encrypt(SendBufferRef& decBuffer, SendBufferRef& encBuffer)
{
	encBuffer = decBuffer;
	return true;
}

/*----------------------------------------------------------------------------*\
|                                                                              |
|                                 TLSSession                                   |
|                                                                              |
\*----------------------------------------------------------------------------*/
TLSSession::TLSSession(ServiceRef service) : Session(service)
{
	if (ServiceRef service = _service.lock())
		_ssl.Init(service->GetSSLContext());
}

void TLSSession::TLSAccept()
{
	SslStatus status = _ssl.Accept();
	uint32 pendingDataSize;

	switch (status)
	{
	case SslStatus::Ok:
		cout << "OK" << endl;
		_recvEvent.SetEventType(EventType::Recv);
		RegisterRecv();
		break;
	case SslStatus::WantRead:
		//wbio에 보낼 데이터가 생겼으면 보내고 recv 등록
		_recvEvent.SetEventType(EventType::TLSHandshakeAcceptRecv);
		pendingDataSize = _ssl.GetWBioPendingSize();
		if (pendingDataSize > 0)
		{
			SendBufferRef sendBuffer = make_shared<SendBuffer>(pendingDataSize);
			uint32 readLen = _ssl.ReadWBio(sendBuffer->GetBuffer(), pendingDataSize);
			sendBuffer->OnWrite(readLen);
			HandshakeSend(sendBuffer);
		}
		RegisterRecv();
		break;
	case SslStatus::WantWrite:
		//wbio가 꽉 차서 Accept가 진행되지 못한 경우. wbio에 있는 데이터를 Send한다.
		pendingDataSize = _ssl.GetWBioPendingSize();
		if (pendingDataSize > 0)
		{
			SendBufferRef sendBuffer = make_shared<SendBuffer>(pendingDataSize);
			uint32 readLen = _ssl.ReadWBio(sendBuffer->GetBuffer(), pendingDataSize);
			sendBuffer->OnWrite(readLen);
			HandshakeSend(sendBuffer);
		}
		break;
	default:
		// 에러 발생함. 연결 종료.
		RegisterDisconnect();
		break;
	}
}

void TLSSession::TLSConnect()
{
	SslStatus status = _ssl.Connect();
	uint32 pendingDataSize;

	switch (status)
	{
	case SslStatus::Ok:
		cout << "OK" << endl;
		pendingDataSize = _ssl.GetWBioPendingSize();
		if (pendingDataSize > 0)
		{
			SendBufferRef sendBuffer = make_shared<SendBuffer>(pendingDataSize);
			uint32 readLen = _ssl.ReadWBio(sendBuffer->GetBuffer(), pendingDataSize);
			sendBuffer->OnWrite(readLen);
			HandshakeSend(sendBuffer);
		}
		_recvEvent.SetEventType(EventType::Recv);
		RegisterRecv();
		break;
	case SslStatus::WantRead:
		//wbio에 보낼 데이터가 생겼으면 보내고 recv 등록
		_recvEvent.SetEventType(EventType::TLSHandshakeConnectRecv);
		pendingDataSize = _ssl.GetWBioPendingSize();
		if (pendingDataSize > 0)
		{
			SendBufferRef sendBuffer = make_shared<SendBuffer>(pendingDataSize);
			uint32 readLen = _ssl.ReadWBio(sendBuffer->GetBuffer(), pendingDataSize);
			sendBuffer->OnWrite(readLen);
			HandshakeSend(sendBuffer);
		}
		RegisterRecv();
		break;
	case SslStatus::WantWrite:
		//wbio가 꽉 차서 Accept가 진행되지 못한 경우. wbio에 있는 데이터를 Send한다.
		pendingDataSize = _ssl.GetWBioPendingSize();
		if (pendingDataSize > 0)
		{
			SendBufferRef sendBuffer = make_shared<SendBuffer>(pendingDataSize);
			uint32 readLen = _ssl.ReadWBio(sendBuffer->GetBuffer(), pendingDataSize);
			sendBuffer->OnWrite(readLen);
			HandshakeSend(sendBuffer);
		}
		break;
	default:
		// 에러 발생함. 연결 종료.
		RegisterDisconnect();
		break;
	}
}

// 
void TLSSession::ProcessTLSHandshakeAcceptRecv(int32 numOfBytes)
{
	_recvEvent.Clear();
	if (numOfBytes == 0) // 클라이언트가 정상적으로 연결을 종료한 경우
	{
		RegisterDisconnect();
		return;
	}
	RecvBuffer& encBuffer = GetEncRecvBuffer();
	encBuffer.OnWrite(numOfBytes);

	uint32 wlen = _ssl.WriteRBio(encBuffer.ReadPos(), numOfBytes);
	encBuffer.OnRead(wlen);
	encBuffer.Clean();

	TLSAccept();
}

void TLSSession::ProcessTLSHandshakeConnectRecv(int32 numOfBytes)
{
	_recvEvent.Clear();
	if (numOfBytes == 0) // 서버가 정상적으로 연결을 종료한 경우
	{
		RegisterDisconnect();
		return;
	}
	RecvBuffer& encBuffer = GetEncRecvBuffer();
	encBuffer.OnWrite(numOfBytes);

	uint32 wlen = _ssl.WriteRBio(encBuffer.ReadPos(), numOfBytes);
	encBuffer.OnRead(wlen);
	encBuffer.Clean();

	TLSConnect();
}

// enc버퍼의 암호문을 복호화 해서 dec버퍼에 넣는 함수.
// 리턴 0 성공. 1 shutdown, 2 에러
uint8 TLSSession::Decrypt(RecvBuffer& encBuffer, RecvBuffer& decBuffer)
{
	uint32 wlen = _ssl.WriteRBio(encBuffer.ReadPos(), encBuffer.DataSize());
	encBuffer.OnRead(wlen);

	size_t recvSize;
	SslStatus status = _ssl.Read(decBuffer.ReadPos(), decBuffer.FreeSize(), &recvSize);
	switch (status)
	{
	case SslStatus::Ok:
		decBuffer.OnWrite(recvSize);
		return 0;
	case SslStatus::WantRead://복호화 하기에 데이터 부족함.
		return 0;
	case SslStatus::Shutdown://상대가 shutdown함. shutdown 호출 가능.
		return 1;
	default:// 에러 발생. shutdown 호출 불가.
		return 2;
	}
}

// dec버퍼의 평문을 암호화 해서 enc버퍼에 넣는 함수.
bool TLSSession::Encrypt(SendBufferRef& decBuffer, SendBufferRef& encBuffer)
{
	size_t writtenLen;
	SslStatus status = _ssl.Write(decBuffer->GetBuffer(), decBuffer->GetDataLen(), &writtenLen);
	if (status == SslStatus::Fail)
		return false;
	uint32 pendingSize = _ssl.GetWBioPendingSize();
	if (pendingSize == 0)
		return false;
	SendBufferRef sendBuffer = make_shared<SendBuffer>(pendingSize);
	uint32 rlen = _ssl.ReadWBio(sendBuffer->GetBuffer(), pendingSize);
	sendBuffer->OnWrite(rlen);
	encBuffer = sendBuffer;
	return true;
}

void TLSSession::HandshakeSend(SendBufferRef sendBuffer)
{
	{
		lock_guard<mutex> lock(_m);
		_sendBuffers.push(sendBuffer);
	}

	bool expected = false;
	if (_sendRegistered.compare_exchange_strong(expected, true))
	{
		RegisterSend();
	}
}

/*----------------------------------------------------------------------------*\
|                                                                              |
|                                PacketSession                                 |
|                                                                              |
\*----------------------------------------------------------------------------*/
uint32 PacketSession::OnRecv(BYTE* buffer, uint32 len)
{
	uint32 processLen = 0;
	while (true)
	{
		uint32 dataLen = len - processLen;
		if (dataLen < sizeof(PacketHeader))
			break;

		PacketHeader* header = reinterpret_cast<PacketHeader*>(&buffer[processLen]);
		if (dataLen < header->size)
			break;

		OnRecvPacket(&buffer[processLen], header->size);

		processLen += header->size;
	}
	return processLen;
}
