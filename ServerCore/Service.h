#pragma once

#include "IocpCore.h"
#include "NetAddress.h"
#include <memory>

class Service : public std::enable_shared_from_this<Service>
{
public:
	Service(NetAddress listenerAddr);
	~Service();

	void Start();

	IocpCoreRef GetIocpCore() { return _iocpCore; }
	NetAddress GetListenerAddr() { return _listenerAddr; }
private:
	IocpCoreRef _iocpCore;
	NetAddress _listenerAddr;
};
