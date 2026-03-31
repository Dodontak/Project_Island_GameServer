#pragma once

#include "IocpCore.h"
#include "NetAddress.h"
#include <mutex>
#include <memory>

class Service : public std::enable_shared_from_this<Service>
{
public:
	Service(NetAddress netAddr);
	~Service();

	virtual void Start() abstract;

	SessionRef CreateSession();

	IocpCoreRef GetIocpCore() { return _iocpCore; }
	NetAddress GetAddr() { return _netAddress; }

	void broad_cast_test(SendBufferRef sendBuffer);
	void didconnect_all_test();

	void AddSession(SessionRef session);
	void RemoveSession(SessionRef session);
protected:
	mutex _m;
	IocpCoreRef _iocpCore;
	NetAddress _netAddress;

	set<SessionRef> _sessions;
};

class ServerService : public Service
{
public:
	ServerService(NetAddress listenerAddr);
	virtual void Start() override;

};

class ClientService : public Service
{
public:
	ClientService(NetAddress serverAddr, int32 clientCount);

	virtual void Start() override;
private:
	int32 _clientCount;
};