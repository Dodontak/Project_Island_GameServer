#include "pch.h"
#include "NetAddress.h"
#include <ws2tcpip.h>

NetAddress::NetAddress(string ip, int port)
{
	::memset(&_sockAddr, 0, sizeof(_sockAddr));
	_sockAddr.sin_family = AF_INET;
	::inet_pton(AF_INET, ip.c_str(), &_sockAddr.sin_addr);
	_sockAddr.sin_port = ::htons(port);
}

NetAddress::NetAddress(const NetAddress& addr)
{
	_sockAddr = addr._sockAddr;
}

NetAddress::~NetAddress()
{
}

string NetAddress::GetIp()
{
	char buff[INET_ADDRSTRLEN];
	return ::inet_ntop(AF_INET, &_sockAddr.sin_addr, buff, sizeof(buff));
}