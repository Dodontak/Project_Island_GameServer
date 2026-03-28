#pragma once

#include "Types.h"
#include "IocpCore.h"
#include "NetAddress.h"

class RecvEvent;

class Session : public IocpObject
{
	friend class Listener;
	enum { BUFFER_SIZE = 1024 };
public:
	Session(SOCKET socket);
	virtual ~Session();
public:
	virtual HANDLE GetHandle() override { return (HANDLE)_socket; }
	virtual void Dispatch(int32 numOfBytes, IocpEvent* event) override;

public:
	void RegisterRecv(IocpEvent* recvEvent);
	void ProcessRecv(int32 numOfBytes, RecvEvent* recvEvent);

	bool SetAddressFromAcceptBuffer();

	BYTE* GetRecvBuffer() { return _recvBuffer; }
	BYTE* GetSendBuffer() { return _sendBuffer; }
private:
	SOCKET _socket = INVALID_SOCKET;
	NetAddress _address;

	BYTE _recvBuffer[BUFFER_SIZE];
	BYTE _sendBuffer[BUFFER_SIZE];
};

