#pragma once

#include "IocpCore.h"
#include "NetAddress.h"
#include <mutex>
#include <memory>
#include <functional>
#include <openssl/ssl.h>

using SessionFactory = function<SessionRef(SOCKET)>;

class Service : public std::enable_shared_from_this<Service>
{
public:
	Service(NetAddress netAddr, SessionFactory sessionFactory);
	~Service();

	virtual void Start() abstract;

	SessionRef CreateSession();
	SSL* CreateSSL();

	IocpCoreRef GetIocpCore() { return _iocpCore; }
	NetAddress GetAddr() { return _netAddress; }

	void broad_cast_test(SendBufferRef sendBuffer);
	void didconnect_all_test();

	void AddSession(SessionRef session);
	void RemoveSession(SessionRef session);
protected:
	mutex _m;
	IocpCoreRef _iocpCore;
	SSL_CTX* _ctx;
	NetAddress _netAddress;

	set<SessionRef> _sessions;
	SessionFactory _sessionFactory;
};

class ServerService : public Service
{
public:
	ServerService(NetAddress listenerAddr, SessionFactory sessionFactory,
		const char* certFile, const char* keyFile);
	virtual void Start() override;

};

class ClientService : public Service
{
public:
	ClientService(NetAddress serverAddr, SessionFactory sessionFactory, int32 clientCount);

	virtual void Start() override;
private:
	int32 _clientCount;
};