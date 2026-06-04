#pragma once

#include "JobQueue.h"
#include "Player.h"

namespace Protocol
{
	class GC_MOVE;
}

class Room : public JobQueue
{
public:
	Room();
	~Room();

	void Update();

	void Move(uint64 objectId, const Protocol::Position& dest, float speed);

	void Enter(PlayerRef player, int32 characterIndex, int32 roomId);
	void Leave(PlayerRef player);

	void AddObject(ObjectRef object);

	void Broadcast(SendBufferRef sendBuffer);
private:
	uint32 _roomId;
	map<uint64, PlayerRef> _players;
	map<uint64, ObjectRef> _objects;
};

extern RoomRef GRoom;