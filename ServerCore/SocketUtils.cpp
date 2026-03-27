#include "pch.h"
#include "SocketUtils.h"

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "mswsock.lib")

WSADATA				SocketUtils::wsaData;

LPFN_CONNECTEX		SocketUtils::ConnectEx = nullptr;
LPFN_DISCONNECTEX	SocketUtils::DisconnectEx = nullptr;
LPFN_ACCEPTEX		SocketUtils::AcceptEx = nullptr;

bool SocketUtils::Init()
{
	ASSERT_CRASH(!WSAStartup(MAKEWORD(2, 2), &wsaData));

	ASSERT_CRASH(LoadExtensionFunction(WSAID_ACCEPTEX, reinterpret_cast<LPVOID*>(&AcceptEx)) == 0);
	ASSERT_CRASH(LoadExtensionFunction(WSAID_CONNECTEX, reinterpret_cast<LPVOID*>(&ConnectEx)) == 0);
	ASSERT_CRASH(LoadExtensionFunction(WSAID_DISCONNECTEX, reinterpret_cast<LPVOID*>(&DisconnectEx)) == 0);
	return true;
}

void SocketUtils::Clear()
{
	::WSACleanup();
}

SOCKET SocketUtils::CreateSocket()
{
	return WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
}

void SocketUtils::CloseSocket(SOCKET sock)
{
	if (sock == INVALID_SOCKET)
		return;
	closesocket(sock);
	sock = INVALID_SOCKET;
}

int SocketUtils::BindSocket(int sock)
{
	return 0;
}

int SocketUtils::ListenSocket(int sock)
{
	return 0;
}

bool SocketUtils::SetReuseAddress(SOCKET socket, bool flag)
{
	return SetSockOpt(socket, SOL_SOCKET, SO_REUSEADDR, flag);
}

bool SocketUtils::SetTcpNoDelay(SOCKET socket, bool flag)
{
	return SetSockOpt(socket, SOL_SOCKET, TCP_NODELAY, flag);
}

int32 SocketUtils::LoadExtensionFunction(GUID guid, LPVOID* func)
{
	SOCKET dummySocket = CreateSocket();
	if (dummySocket == INVALID_SOCKET)
		return false;
	DWORD bytes = 0;
	int32 ret = WSAIoctl(dummySocket, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), func, sizeof(*func), &bytes, NULL, NULL);
	CloseSocket(dummySocket);
	return ret;
}