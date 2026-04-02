#include "pch.h"
#include "ServerSession.h"

void ServerSession::OnRecvPacket(BYTE* buffer, uint32 size)
{
	uint16 headerSize = sizeof(PacketHeader);
	string str((char*)(buffer + headerSize), size - headerSize);
	Utils::LockPrint("recv from server : ", str);
}
