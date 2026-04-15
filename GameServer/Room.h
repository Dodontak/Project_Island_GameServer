#pragma once

#include "Player.h"

class Room
{
public:
	Room();
	~Room();

	void Enter(PlayerRef player);
	void Leave(PlayerRef player);
	void Broadcast(SendBufferRef sendBuffer);
private:
	mutex _m;
	uint32 _roomId;
	map<uint64, PlayerRef> _players;
};

extern Room GRoom[2];