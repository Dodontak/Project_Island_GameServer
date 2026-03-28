#pragma once

#include "IocpCore.h"
#include "IocpEvent.h"
#include "NetAddress.h"

class Listener : public IocpObject
{
public:
	Listener(ServiceRef service);
	~Listener();
public:
	virtual HANDLE GetHandle() override { return (HANDLE)_listenSocket; }
	virtual void Dispatch(int32 numOfBytes, IocpEvent* event) override;

public:
	NetAddress GetAddress() { return _address; }
public:
	bool StartAccept();

	void RegisterAccept(AcceptEvent* acceptEvent);
	void ProcessAccept(SessionRef session);
private:


private:
	SOCKET _listenSocket = INVALID_SOCKET;
	ServiceRef _service;
	NetAddress _address;
};

