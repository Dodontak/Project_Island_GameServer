#include "pch.h"
#include "GameSession.h"
#include "Service.h"


void GameSession::OnRecvPacket(BYTE* buffer, uint32 size)
{
	uint16 headerSize = sizeof(PacketHeader);
	SendBufferRef sendBuffer = Utils::MakeChatSendBuffer(1, buffer + headerSize, size - headerSize);
	if (ServiceRef service = _service.lock())
	{
		service->broad_cast_test(sendBuffer);
	}
}
