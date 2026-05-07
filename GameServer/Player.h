#pragma once
#include "Protocol.pb.h"

class Room;

class Player
{
public:
	Player(const Protocol::PlayerInfo& player, GameSessionRef owner);
	~Player();

	void ChatTest(const string& msg);

	Protocol::PlayerInfo _info;
	weak_ptr<GameSession> _owner;
	weak_ptr<Room> _room;
};

