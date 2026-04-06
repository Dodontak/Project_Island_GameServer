#include "pch.h"
#include "Session.h"
#include "IocpEvent.h"
#include "Service.h"
#include "SendBuffer.h"

Session::Session(SOCKET socket) : _socket(socket), _recvBuffer(BUFFER_SIZE)
{
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
			_recvBuffer.Clean();
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
	uint32 decryptedLen = 0;
	GetEncRecvBuffer().OnWrite(numOfBytes);
	do
	{
		decryptedLen += OnDecrypt(GetEncRecvBuffer(), GetDecRecvBuffer());
		//EncRecvBuffer OnRead는 OnDecrypt 내부에서 처리함.
		GetDecRecvBuffer().OnWrite(decryptedLen);
	} while (HasPendingData());

	int processLen = OnRecv(GetDecRecvBuffer().ReadPos(), GetDecRecvBuffer().DataSize());
	GetDecRecvBuffer().OnRead(processLen);
	
	GetEncRecvBuffer().Clean();
	GetDecRecvBuffer().Clean();

	RegisterRecv();
}

/*----------------------------------------------------------------------------*\
|                                  Send                                        |
\*----------------------------------------------------------------------------*/
void	 Session::Send(SendBufferRef sendBuffer)
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
	{
		service->AddSession(static_pointer_cast<Session>(shared_from_this()));
	}
	RegisterRecv();
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

int32 Session::OnDecrypt(RecvBuffer& encrypt, RecvBuffer& decrypt)
{
	memcpy(decrypt.WritePos(), encrypt.ReadPos(), encrypt.DataSize());
	encrypt.OnRead(encrypt.DataSize());
	return encrypt.DataSize();
}

/*----------------------------------------------------------------------------*\
|                                                                              |
|                                 TLSSession                                   |
|                                                                              |
\*----------------------------------------------------------------------------*/

TLSSession::TLSSession(SOCKET socket) : Session(socket), _encryptedRecvBuffer(BUFFER_SIZE)
{
	if (ServiceRef service = _service.lock())
	{
		_ssl = service->CreateSSL();
		_rbio = BIO_new(BIO_s_mem());
		_wbio = BIO_new(BIO_s_mem());
		if (_rbio == nullptr || _wbio == nullptr)
			Utils::HandleError("Failed to create BIO in TLSSession constructor");
		SSL_set_bio(_ssl, _rbio, _wbio);
	}
}

int32 TLSSession::OnDecrypt(RecvBuffer& encrypt, RecvBuffer& decrypt)
{
	int32 wlen = BIO_write(_rbio, encrypt.ReadPos(), encrypt.DataSize());
	if (wlen <= 0);
		//TODO BIO_write 실패 처리
	encrypt.OnRead(wlen);
	int32 rlen = SSL_read(_ssl, decrypt.WritePos(), decrypt.FreeSize());
	if (rlen < 0);
		//TODO SSL_read 실패 처리
	return rlen;
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
