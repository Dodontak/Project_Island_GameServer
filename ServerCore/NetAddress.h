#pragma once

class NetAddress
{
public:
	NetAddress(string ip, int port);
	NetAddress(const NetAddress& addr);
	~NetAddress();

	const SOCKADDR_IN& GetAddr() { return _sockAddr; }
	string GetIp();
	int16 GetPort() { return ::ntohs(_sockAddr.sin_port); }
private:
	SOCKADDR_IN _sockAddr;
};

