#include "pch.h"
#include "ServerPacketHandler.h"
#include "ServerSession.h"

function<bool(function<void()>&, PacketSessionRef&, BYTE*, int32)> GPacketHandler[UINT16_MAX];

bool Handle_INVALID(function<void()>& outFunc, PacketSessionRef& session, BYTE* buffer, int32 len)
{
	return false;
}

void Handle_S_LOGIN(const PacketSessionRef& session, const Protocol::S_LOGIN& pkt)
{
	if (pkt.success() == false)
	{
		// TODO 로그인 실패 처리
		return;
	}

	ServerSessionRef serverSession = static_pointer_cast<ServerSession>(session);
	serverSession->_userId = pkt.user_id();

	//로그인 성공하면 바로 room 입장 패킷 전송.
	//room은 짝수방(0), 홀수방(1) 만있다고 가정.
	Protocol::C_ENTER_ROOM enterRoomPkt;

	enterRoomPkt.set_room_id(pkt.user_id() % 2); // 짝수방(0), 홀수방(1) 만있다고 가정
	session->Send(ServerPacketHandler::MakeSendBuffer(enterRoomPkt));
}

void Handle_S_ENTER_ROOM(const PacketSessionRef& session, const Protocol::S_ENTER_ROOM& pkt)
{
	if (pkt.success() == false)
	{
		// TODO 방 입장 실패 처리
		return;
	}
	ServerSessionRef serverSession = static_pointer_cast<ServerSession>(session);
	Utils::LockPrint(serverSession->_userId, "enter room success");
}

void Handle_S_CHAT(const PacketSessionRef& session, const Protocol::S_CHAT& pkt)
{
	ServerSessionRef serverSession = static_pointer_cast<ServerSession>(session);
	Utils::LockPrint(pkt.msg());
}