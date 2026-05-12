#include "pch.h"
#include "ClientPacketHandler.h"
#include "Utils.h"
#include "GameSession.h"
#include "Player.h"
#include "Room.h"
#include "JobTimer.h"

PacketHandlerFunc GPacketHandler[UINT16_MAX];

bool	Handle_INVALID(DeferredFunc& outFunc, PacketSessionRef& session, BYTE* buffer, int32 len)
{
	return false;
}

void Handle_GC_LOGIN(const PacketSessionRef& session, const Protocol::GC_LOGIN& pkt)
{
	Protocol::GS_LOGIN response;

	string jwt = pkt.jwt();
	string userId;
	string nickname;

	if (false == Utils::VerifyAccessToken(jwt, OUT userId, OUT nickname))
	{// TODO 인증 실패
		response.set_success(false);
		session->Send(ClientPacketHandler::MakeSendBuffer(response));
		return;
	}
	response.set_success(true);
	response.set_user_id(stoi(userId));

	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);
	gameSession->_userId = stoi(userId);
	gameSession->Send(ClientPacketHandler::MakeSendBuffer(response));
}

void Handle_GC_ENTER_ROOM(const PacketSessionRef& session, const Protocol::GC_ENTER_ROOM& pkt)
{
	Protocol::GS_ENTER_ROOM response;

	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

	Protocol::PlayerInfo playerInfo;
	playerInfo.set_id(gameSession->_userId);
	playerInfo.set_name("Player" + ::to_string(gameSession->_userId));
	switch (gameSession->_userId % 4)
	{
	case 0:
		playerInfo.set_playertype(Protocol::PlayerType::PLAYER_TYPE_ARCHER);
		break;
	case 1:
		playerInfo.set_playertype(Protocol::PlayerType::PLAYER_TYPE_KNIGHT);
		break;
	case 2:
		playerInfo.set_playertype(Protocol::PlayerType::PLAYER_TYPE_MAGE);
		break;
	}
	Protocol::Position pos;
	pos.set_x(Utils::GetRandNum(0, 1000));
	pos.set_y(Utils::GetRandNum(0, 1000));
	pos.set_x(100);
	playerInfo.mutable_pos()->CopyFrom(pos);

	PlayerRef player = make_shared<Player>(playerInfo, gameSession);
	gameSession->_player = player;
	RoomRef room = GRoom[pkt.room_id()];

	room->DoAsync(&Room::Enter, player);

	response.set_success(true);
	this_thread::sleep_for(chrono::seconds(1));
	session->Send(ClientPacketHandler::MakeSendBuffer(response));
}

void Handle_GC_LEAVE_ROOM(const PacketSessionRef& session, const Protocol::GC_LEAVE_ROOM& pkt)
{
}

void	Handle_GC_CHAT(const PacketSessionRef& session, const Protocol::GC_CHAT& pkt)
{
	Utils::LockPrint("message from client:", pkt.msg());
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);
	if (gameSession->_player == nullptr)
		return;
	gameSession->_player->ChatTest(pkt.msg());
}