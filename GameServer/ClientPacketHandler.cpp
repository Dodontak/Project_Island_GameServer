#include "pch.h"
#include "ClientPacketHandler.h"

function<bool(function<void()>&, PacketSessionRef&, BYTE*, int32)> GPacketHandler[UINT16_MAX];

bool	Handle_INVALID(std::function<void()>& outFunc, PacketSessionRef& session, BYTE* buffer, int32 len)
{
	return false;
}

void	Handle_C_CHAT(const PacketSessionRef& session, const Protocol::C_CHAT& pkt)
{

}