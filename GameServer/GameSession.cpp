#include "pch.h"
#include "GameSession.h"
#include "Service.h"
#include "ClientPacketHandler.h"

GameSession::GameSession(ServiceRef service) : PacketSession(service)
{
	// 임시 유저id. 실제로는 jwt로부터 유저id 추출해서 할당해야됨.
	static atomic<uint32> s_userId = 1;
	_userId = s_userId.fetch_add(1);
}

void GameSession::OnRecvPacket(BYTE* buffer, uint32 size)
{
	function<void()> func;
	ClientPacketHandler::PacketHandler(func, static_pointer_cast<PacketSession>(shared_from_this()),
		buffer, size);
	func();
}

void GameSession::broad_cast_test(SendBufferRef sendBuffer)
{
	if (ServiceRef service = _service.lock())
	{
		service->broad_cast_test(sendBuffer);
	}
}

