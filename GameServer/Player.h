#pragma once
#include "Protocol.pb.h"

class Room;

class Player
{
public:
	Player(const Protocol::Player& player, GameSessionRef owner);
	~Player();

	void ChatTest(const string& msg);

	Protocol::Player _info;
	weak_ptr<GameSession> _owner;
	weak_ptr<Room> _room;
};

