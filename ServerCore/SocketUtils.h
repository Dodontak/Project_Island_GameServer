#pragma once

#include <winsock2.h>
#include <mswsock.h>

class SocketUtils
{
public:
	static LPFN_CONNECTEX ConnectEx;
	static LPFN_DISCONNECTEX DisconnectEx;
	static LPFN_ACCEPTEX AcceptEx;

public:
	static bool Init();
	static void Clear();

	static SOCKET CreateSocket();
	static void CloseSocket(SOCKET sock);
	static int BindSocket(int sock);
	static int ListenSocket(int sock);
	static bool SetReuseAddress(SOCKET socket, bool flag);
	static bool SetTcpNoDelay(SOCKET socket, bool flag);

private:
	static int32 LoadExtensionFunction(GUID guid, LPVOID* func);
	static WSADATA wsaData;

	template<typename T>
	static bool SetSockOpt(SOCKET socket, int32 level, int32 optName, T optVal)
	{
		return SOCKET_ERROR != ::setsockopt(socket, level, optName, reinterpret_cast<char*>(&optVal), sizeof(T));
	}
};