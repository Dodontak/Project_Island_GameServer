#include "pch.h"
#include "ServerSession.h"
#include "ServerPacketHandler.h"

void ServerSession::OnRecvPacket(BYTE* buffer, uint32 size)
{
	function<void()> func;
	ServerPacketHandler::PacketHandler(func, static_pointer_cast<PacketSession>(shared_from_this()),
		buffer, size);
	func();
}

void ServerSession::OnConnect()
{
	Protocol::C_LOGIN pkt;

	// TODO 인증서버로 부터 받은 jwt를 게임서버로 전달.
	pkt.set_jwt("pass");
	Send(ServerPacketHandler::MakeSendBuffer(pkt));
}
