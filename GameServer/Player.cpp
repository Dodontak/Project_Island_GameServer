#include "pch.h"
#include "Player.h"
#include "Room.h"
#include "GameSession.h"
#include "ClientPacketHandler.h"

Player::Player(const Protocol::PlayerInfo& player, GameSessionRef owner) : _owner(owner)
{
	_info.CopyFrom(player);
}

Player::~Player() {}

void Player::ChatTest(const string& msg)
{
	if (RoomRef room = _room.lock())
	{
		Protocol::GS_CHAT pkt;
		string chatMsg = _info.name() + " : " + msg;
		pkt.set_msg(chatMsg);

		room->DoAsync(&Room::Broadcast, ClientPacketHandler::MakeSendBuffer(pkt));
	}
}
