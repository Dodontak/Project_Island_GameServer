#pragma once

#include "Session.h"

class ServerSession : public PacketSession
{
public:
	ServerSession(SOCKET socket) : PacketSession(socket) {}
	~ServerSession() {}

	virtual void OnRecvPacket(BYTE* buffer, uint32 size) override;
};

