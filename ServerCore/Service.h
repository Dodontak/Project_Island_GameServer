#pragma once

#include "IocpCore.h"
#include "NetAddress.h"
#include <mutex>
#include <memory>

class Service : public std::enable_shared_from_this<Service>
{
public:
	Service(NetAddress listenerAddr);
	~Service();

	void Start();

	SessionRef CreateSession();

	IocpCoreRef GetIocpCore() { return _iocpCore; }
	NetAddress GetListenerAddr() { return _listenerAddr; }

	void broad_cast_test(SendBufferRef sendBuffer);
	void AddSession(SessionRef session);
	void RemoveSession(SessionRef session);
private:
	mutex _m;
	IocpCoreRef _iocpCore;
	NetAddress _listenerAddr;

	set<SessionRef> _sessions;
};
