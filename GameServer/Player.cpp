#include "pch.h"
#include "Player.h"
Player::Player(const Protocol::Player& player, GameSessionRef owner) : _owner(owner)
{
	_info.CopyFrom(player);
}

Player::~Player()
{
}