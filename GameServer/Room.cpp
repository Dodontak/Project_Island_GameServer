#include "pch.h"
#include "Room.h"
#include "Session.h"
#include "ClientPacketHandler.h"
#include "GameSession.h"

Room GRoom[2];

Room::Room()
{
	static atomic<uint32> s_roomId = 0;
	_roomId = s_roomId.fetch_add(1);
}

Room::~Room() {}

void Room::Enter(PlayerRef player)
{
	lock_guard<mutex> lock(_m);
	_players.insert({ player->_info.id(), player });

	Protocol::S_CHAT pkt;
	string msg = player->_info.name() + " Entered Room" + to_string(_roomId);
	pkt.set_msg(msg);

	Broadcast(ClientPacketHandler::MakeSendBuffer(pkt));
}

void Room::Leave(PlayerRef player)
{
	lock_guard<mutex> lock(_m);

	Protocol::S_CHAT pkt;

	string msg = "Room " + to_string(_roomId) + " : " + player->_info.name() + " Left Room.";
	pkt.set_msg(msg);

	_players.erase(player->_info.id());
	Broadcast(ClientPacketHandler::MakeSendBuffer(pkt));
}

void Room::Broadcast(SendBufferRef sendBuffer)
{
	for (auto& player : _players)
	{
		player.second->_owner->Send(sendBuffer);
	}
}
