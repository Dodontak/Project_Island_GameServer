#pragma once

class NetAddress
{
public:
	NetAddress() = default;
	NetAddress(string ip, int port);
	NetAddress(const NetAddress& addr);
	NetAddress(SOCKADDR_IN addr);
	~NetAddress();

	void SetAddr(const NetAddress& addr) { _sockAddr = addr._sockAddr; }

	const SOCKADDR_IN&	GetAddr() { return _sockAddr; }
	string				GetIp();
	uint16				GetPort() { return ::ntohs(_sockAddr.sin_port); }
private:
	SOCKADDR_IN _sockAddr;
};