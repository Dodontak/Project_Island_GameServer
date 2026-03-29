#pragma once

#include <queue>
#include "Types.h"
#include "IocpCore.h"
#include "NetAddress.h"
#include "IocpEvent.h"
#include "RecvBuffer.h"
#include "SendBuffer.h"

class Session : public IocpObject
{
	enum { BUFFER_SIZE = 0x10000 };// 64KB
	friend class Listener;
	friend class Service;
public:
	Session(SOCKET socket);
	virtual ~Session();
public:
	virtual HANDLE GetHandle() override { return (HANDLE)_socket; }
	virtual void Dispatch(int32 numOfBytes, IocpEvent* event) override;

public:
	void RegisterRecv();
	void ProcessRecv(int32 numOfBytes);

	void RegisterSend();
	void ProcessSend(int32 numOfBytes);

	void RegisterDisconnect();
	void ProcessDisconnect();

	bool SetAddressFromAcceptBuffer(BYTE* buffer);

	NetAddress GetAddr() { return _address; }
private:
	weak_ptr<Service> _service;
	SOCKET _socket = INVALID_SOCKET;
	NetAddress _address;

	RecvBuffer _recvBuffer;
	queue<SendBufferRef> _sendBuffers;

private:
	RecvEvent _recvEvent;
	SendEvent _sendEvent;
	ConnectEvent _connectEvent;
	DisconnectEvent _disconnectEvent;
};

