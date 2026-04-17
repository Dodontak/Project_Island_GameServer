#include "pch.h"
#include "Room.h"
#include "Player.h"
#include "Session.h"
#include "ClientPacketHandler.h"
#include "GameSession.h"

RoomRef GRoom[2];

Room::Room()
{
	static atomic<uint32> s_roomId = 0;
	_roomId = s_roomId.fetch_add(1);
}

Room::~Room() {}

void Room::Enter(PlayerRef player)
{
	_players.insert({ player->_info.id(), player });

	Protocol::S_CHAT pkt;
	player->_room = static_pointer_cast<Room>(shared_from_this());
	string msg = player->_info.name() + " Entered Room" + to_string(_roomId);
	pkt.set_msg(msg);

	Broadcast(ClientPacketHandler::MakeSendBuffer(pkt));
}

void Room::Leave(PlayerRef player)
{
	_players.erase(player->_info.id());
	
	Protocol::S_CHAT pkt;
	string msg = "Room " + to_string(_roomId) + " : " + player->_info.name() + " Left Room.";
	pkt.set_msg(msg);
	Broadcast(ClientPacketHandler::MakeSendBuffer(pkt));
}

void Room::Broadcast(SendBufferRef sendBuffer)
{
	for (auto& player : _players)
	{
		if (auto owner = player.second->_owner.lock())
		{
			owner->Send(sendBuffer);
		}
	}
}
