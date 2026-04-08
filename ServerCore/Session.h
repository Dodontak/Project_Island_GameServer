#pragma once

#include <queue>
#include <mutex>
#include <openssl/ssl.h>
#include <openssl/bio.h>
#include "Types.h"
#include "IocpCore.h"
#include "NetAddress.h"
#include "IocpEvent.h"
#include "RecvBuffer.h"
#include "SendBuffer.h"
#include "SslObject.h"

/*----------------------------------------------------------------------------*\
|                                                                              |
|                                   Session                                    |
|                                                                              |
\*----------------------------------------------------------------------------*/
class Session : public IocpObject
{
	friend class Listener;
	friend class Service;
protected:
	enum { BUFFER_SIZE = 0x10000 };// 64KB

public:
	Session(ServiceRef service);
	virtual ~Session();

	virtual uint32 OnRecv(BYTE* buffer, uint32 len) { return len; }
public:
	virtual HANDLE GetHandle() override { return (HANDLE)_socket; }
	virtual void Dispatch(int32 numOfBytes, IocpEvent* event) override;

public:
	void RegisterRecv();
	void ProcessRecv(int32 numOfBytes);

	void Send(SendBufferRef sendBuffer);
	void RegisterSend();
	void ProcessSend(int32 numOfBytes);

	void RegisterConnect();
	void ProcessConnect();

	void RegisterDisconnect();
	void ProcessDisconnect();

	bool SetAddressFromAcceptBuffer(BYTE* buffer);

	void SetAddr(const NetAddress& address) { _address = address; }
	NetAddress GetAddr() { return _address; }
protected:
	mutex _m;
	atomic<bool> _sendRegistered = false;
	atomic<bool> _isConnected = false;

	weak_ptr<Service> _service;
	SOCKET _socket = INVALID_SOCKET;
	NetAddress _address;

	RecvBuffer _recvBuffer;
	queue<SendBufferRef> _sendBuffers;

	RecvBuffer& GetDecRecvBuffer() { return _recvBuffer; }
	virtual RecvBuffer& GetEncRecvBuffer() { return _recvBuffer; }

protected:
	RecvEvent _recvEvent;
	SendEvent _sendEvent;
	ConnectEvent _connectEvent;
	DisconnectEvent _disconnectEvent;
};

/*----------------------------------------------------------------------------*\
|                                                                              |
|                                 TLSSession                                   |
|                                                                              |
\*----------------------------------------------------------------------------*/
class TLSSession : public Session
{
public:
	TLSSession(ServiceRef service);
protected:
	SslObject _ssl;
};

/*----------------------------------------------------------------------------*\
|                                                                              |
|                                PacketSession                                 |
|                                                                              |
\*----------------------------------------------------------------------------*/
struct PacketHeader
{
	uint16 id;
	uint16 size;
};

class PacketSession : public TLSSession
{
public:
	PacketSession(ServiceRef service) : TLSSession(service) {}
	virtual ~PacketSession() {}

	virtual uint32 OnRecv(BYTE* buffer, uint32 len) sealed;
	virtual void OnRecvPacket(BYTE* buffer, uint32 size) abstract;
};