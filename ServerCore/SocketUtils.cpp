#include "pch.h"
#include "SocketUtils.h"
#include "NetAddress.h"

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "mswsock.lib")

WSADATA				SocketUtils::wsaData;
//BOOL LpfnConnectex(
//	[in]           SOCKET s,
//	[in]           const sockaddr* name,
//	[in]           int namelen,
//	[in, optional] PVOID lpSendBuffer,
//	[in]           DWORD dwSendDataLength,
//	[out]          LPDWORD lpdwBytesSent,
//	[in]           LPOVERLAPPED lpOverlapped
//)
LPFN_CONNECTEX		SocketUtils::ConnectEx = nullptr;
//BOOL LpfnDisconnectex(
//	SOCKET s,
//	LPOVERLAPPED lpOverlapped,
//	DWORD dwFlags,
//	DWORD dwReserved
//)
LPFN_DISCONNECTEX	SocketUtils::DisconnectEx = nullptr;
//BOOL AcceptEx(
//	[in]  SOCKET       sListenSocket,
//	[in]  SOCKET       sAcceptSocket,
//	[in]  PVOID        lpOutputBuffer,
//	[in]  DWORD        dwReceiveDataLength,
//	[in]  DWORD        dwLocalAddressLength,
//	[in]  DWORD        dwRemoteAddressLength,
//	[out] LPDWORD      lpdwBytesReceived,
//	[in]  LPOVERLAPPED lpOverlapped
//);
LPFN_ACCEPTEX		SocketUtils::AcceptEx = nullptr;
LPFN_GETACCEPTEXSOCKADDRS SocketUtils::GetAcceptExSockaddrs = nullptr;
bool SocketUtils::Init()
{
	ASSERT_CRASH(!WSAStartup(MAKEWORD(2, 2), &wsaData));

	ASSERT_CRASH(LoadExtensionFunction(WSAID_ACCEPTEX, reinterpret_cast<LPVOID*>(&AcceptEx)) == 0);
	ASSERT_CRASH(LoadExtensionFunction(WSAID_CONNECTEX, reinterpret_cast<LPVOID*>(&ConnectEx)) == 0);
	ASSERT_CRASH(LoadExtensionFunction(WSAID_DISCONNECTEX, reinterpret_cast<LPVOID*>(&DisconnectEx)) == 0);
	ASSERT_CRASH(LoadExtensionFunction(WSAID_GETACCEPTEXSOCKADDRS,
		reinterpret_cast<LPVOID*>(&GetAcceptExSockaddrs)) == 0);
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

void SocketUtils::CloseSocket(SOCKET& sock)
{
	if (sock == INVALID_SOCKET)
		return;
	closesocket(sock);
	sock = INVALID_SOCKET;
}

bool SocketUtils::BindSocket(SOCKET sock, NetAddress address)
{
	if (SOCKET_ERROR == bind(sock, (const sockaddr*)&address.GetAddr(), sizeof(address.GetAddr())))
		return false;
	return true;
}

bool SocketUtils::ListenSocket(SOCKET sock, int32 backlog)
{
	if (listen(sock, backlog) == SOCKET_ERROR)
		return false;
	return true;
}

bool SocketUtils::SetReuseAddress(SOCKET socket, bool flag)
{
	return SetSockOpt(socket, SOL_SOCKET, SO_REUSEADDR, flag);
}

bool SocketUtils::SetTcpNoDelay(SOCKET socket, bool flag)
{
	return SetSockOpt(socket, SOL_SOCKET, TCP_NODELAY, flag);
}

// ListenSocket의 특성을 ClientSocket에 그대로 적용
bool SocketUtils::SetUpdateAcceptSocket(SOCKET socket, SOCKET listenSocket)
{
	return SetSockOpt(socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, listenSocket);
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