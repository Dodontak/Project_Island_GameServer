#pragma once
#include "Protocol.pb.h"

class Room;

class Player : public enable_shared_from_this<Player>
{
public:
	Player(GameSessionRef session);
	virtual ~Player();

	void ChatTest(const string& msg);

	Protocol::PlayerInfo _info;
	weak_ptr<GameSession> _owner;
	atomic<weak_ptr<Room>> _room;
};
