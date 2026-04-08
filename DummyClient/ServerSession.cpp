#include "pch.h"
#include "ServerSession.h"
#include "ServerPacketHandler.h"

void ServerSession::OnRecvPacket(BYTE* buffer, uint32 size)
{
	function<void()> func;
	ServerPacketHandler::PacketHandler(func, static_pointer_cast<PacketSession>(shared_from_this()),
		buffer, size);
	func();
	//uint16 headerSize = sizeof(PacketHeader);
	//string str((char*)(buffer + headerSize), size - headerSize);
	//Utils::LockPrint("recv from server : ", str);
}
