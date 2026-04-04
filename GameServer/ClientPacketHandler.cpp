#include "pch.h"
#include "ClientPacketHandler.h"
#include "Utils.h"
#include "GameSession.h"

function<bool(function<void()>&, PacketSessionRef&, BYTE*, int32)> GPacketHandler[UINT16_MAX];

bool	Handle_INVALID(std::function<void()>& outFunc, PacketSessionRef& session, BYTE* buffer, int32 len)
{
	return false;
}

void	Handle_C_CHAT(const PacketSessionRef& session, const Protocol::C_CHAT& pkt)
{
	Protocol::S_CHAT response;
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

	response.set_msg(pkt.msg());
	gameSession->broad_cast_test(ClientPacketHandler::MakeSendBuffer(response));
}