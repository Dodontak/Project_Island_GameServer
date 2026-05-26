#pragma once

#include "JobQueue.h"
#include "Player.h"

class Room : public JobQueue
{
public:
	Room();
	~Room();

	void Move(uint64 objectId, const Protocol::GC_MOVE& dest);

	void Enter(PlayerRef player, int32 characterIndex, int32 roomId);
	void Leave(PlayerRef player);

	void LoadObjects(PlayerRef player);
	void Broadcast(SendBufferRef sendBuffer);
private:
	uint32 _roomId;
	map<uint64, PlayerRef> _players;
};

extern RoomRef GRoom;