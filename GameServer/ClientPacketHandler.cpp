#include "pch.h"
#include "ClientPacketHandler.h"
#include "Utils.h"
#include "GameSession.h"
#include "Player.h"
#include "Room.h"
#include "JobTimer.h"

function<bool(function<void()>&, PacketSessionRef&, BYTE*, int32)> GPacketHandler[UINT16_MAX];

bool	Handle_INVALID(std::function<void()>& outFunc, PacketSessionRef& session, BYTE* buffer, int32 len)
{
	return false;
}

void Handle_C_LOGIN(const PacketSessionRef& session, const Protocol::C_LOGIN& pkt)
{
	Protocol::S_LOGIN response;

	string jwt = pkt.jwt();
	if (jwt != "pass")
	{// TODO 인증 실패
		response.set_success(false);
		session->Send(ClientPacketHandler::MakeSendBuffer(response));
		return;
	}
	response.set_success(true);

	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);
	// 임시 userId 전달. gameSession 생성순서대로 1 2 3 4...
	// 실제로는 jwt 검증하고 userId 확인해야함.
	response.set_user_id(gameSession->_userId);
	gameSession->Send(ClientPacketHandler::MakeSendBuffer(response));
}

void Handle_C_ENTER_ROOM(const PacketSessionRef& session, const Protocol::C_ENTER_ROOM& pkt)
{
	Protocol::S_ENTER_ROOM response;

	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

	Protocol::Player playerInfo;
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

void	Handle_C_CHAT(const PacketSessionRef& session, const Protocol::C_CHAT& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);
	if (gameSession->_player == nullptr)
		return;
	gameSession->_player->ChatTest(pkt.msg());
}