#pragma once

#include "Session.h"

class ServerSession : public PacketSession
{
public:
	ServerSession(ServiceRef service) : PacketSession(service) {}
	~ServerSession() {}

	virtual void OnRecvPacket(BYTE* buffer, uint32 size) override;
};

