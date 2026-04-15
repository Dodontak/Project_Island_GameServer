#pragma once
#include "Protocol.pb.h"

class Player
{
public:
	Player(const Protocol::Player& player, GameSessionRef owner);
	~Player();

	Protocol::Player _info;
	GameSessionRef _owner;
};

