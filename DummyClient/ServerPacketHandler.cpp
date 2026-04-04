#include "pch.h"
#include "ServerPacketHandler.h"

function<bool(function<void()>&, PacketSessionRef&, BYTE*, int32)> GPacketHandler[UINT16_MAX];

bool	Handle_INVALID(function<void()>& outFunc, PacketSessionRef& session, BYTE* buffer, int32 len)
{
	return false;
}

void	Handle_S_CHAT(const PacketSessionRef& session, const Protocol::S_CHAT& pkt)
{

}