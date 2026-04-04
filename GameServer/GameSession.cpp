#include "pch.h"
#include "GameSession.h"
#include "Service.h"
#include "ClientPacketHandler.h"

void GameSession::OnRecvPacket(BYTE* buffer, uint32 size)
{
	function<void()> func;
	ClientPacketHandler::PacketHandler(func, static_pointer_cast<PacketSession>(shared_from_this()),
		buffer, size);
	func();
	//uint16 headerSize = sizeof(PacketHeader);
	//SendBufferRef sendBuffer = Utils::MakeChatSendBuffer(1, buffer + headerSize, size - headerSize);
	//if (ServiceRef service = _service.lock())
	//{
	//	service->broad_cast_test(sendBuffer);
	//}
}

void GameSession::broad_cast_test(SendBufferRef sendBuffer)
{
	if (ServiceRef service = _service.lock())
	{
		service->broad_cast_test(sendBuffer);
	}
}

