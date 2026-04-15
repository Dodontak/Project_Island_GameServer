#pragma once
#include "Session.h"

class GameSession : public PacketSession
{
public:
	GameSession(ServiceRef service);
	~GameSession() {}

	virtual void OnRecvPacket(BYTE* buffer, uint32 size) override;
public:
	void broad_cast_test(SendBufferRef sendBuffer);
	uint32 _userId;
};

