#pragma once

#include "JobQueue.h"
#include "Player.h"

class Room : public JobQueue
{
public:
	Room();
	~Room();

	void Enter(PlayerRef player);
	void Leave(PlayerRef player);
	void Broadcast(SendBufferRef sendBuffer);
private:
	uint32 _roomId;
	map<uint64, PlayerRef> _players;
};

extern RoomRef GRoom;