#include "pch.h"
#include "ClientPacketHandler.h"
#include "Utils.h"
#include "GameSession.h"
#include "Player.h"
#include "Room.h"
#include "JobTimer.h"
#include "DBConnectionPool.h"

PacketHandlerFunc GPacketHandler[UINT16_MAX];

bool	Handle_INVALID(DeferredFunc& outFunc, PacketSessionRef& session, BYTE* buffer, int32 len)
{
	return false;
}

void Handle_GC_LOGIN(const PacketSessionRef& session, const Protocol::GC_LOGIN& pkt)
{
	Protocol::GS_LOGIN response;
	Utils::LockPrint("Handle_GC_LOGIN");
	string jwt = pkt.jwt();
	string userId;
	string nickname;

	if (false == Utils::VerifyAccessToken(jwt, OUT userId, OUT nickname))
	{// TODO 인증 실패
		response.set_success(false);
		response.set_reason("400 JWT Fail");
		session->Send(ClientPacketHandler::MakeSendBuffer(response));
		return;
	}
	response.set_success(true);
	response.set_user_id(stoi(userId));

	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);
	gameSession->_userId = stoi(userId);
	gameSession->Send(ClientPacketHandler::MakeSendBuffer(response));
}

void Handle_GC_CHARACTER_LIST(const PacketSessionRef& session, const Protocol::GC_CHARACTER_LIST& pkt)
{
	Protocol::GS_CHARACTER_LIST response;
	Utils::LockPrint("Handle_GC_CHARACTER_LIST");
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

	string pgGetCharacterListQuery = "SELECT nickname, level, job_type FROM game.characters WHERE user_id = $1";
	PGConnection* pg = GDBConnectionPool->PopPG();
	pg->AddValue(::to_string(gameSession->_userId));

	if (false == pg->ExecuteSQL(pgGetCharacterListQuery))
	{
		Utils::LockPrint("Failed to execute SQL query:", pgGetCharacterListQuery);
		pg->Clear();
		GDBConnectionPool->Push(&pg);
		response.set_success(false);
		response.set_reason("500 DB Fail");
		session->Send(ClientPacketHandler::MakeSendBuffer(response));
		return;
	}
	uint16 characterCount = pg->GetRowCount();
	for (int i = 0; i < characterCount; ++i)
	{
		Protocol::PlayerInfo* character = new Protocol::PlayerInfo();
		character->set_name(pg->GetValue(i, 0));
		character->set_level(stoi(pg->GetValue(i, 1)));
		character->set_playertype((Protocol::PlayerType)stoi(pg->GetValue(i, 2)));
		response.mutable_characters()->AddAllocated(character);
	}
	pg->Clear();
	GDBConnectionPool->Push(&pg);
	response.set_success(true);
	session->Send(ClientPacketHandler::MakeSendBuffer(response));
}

void Handle_GC_CHECK_NICKNAME(const PacketSessionRef& session, const Protocol::GC_CHECK_NICKNAME& pkt)
{
	Protocol::GS_CHECK_NICKNAME response;
	Utils::LockPrint("Handle_GC_CHECK_NICKNAME");

	string pgCheckNicknameQuery = "SELECT EXISTS(SELECT 1 FROM game.characters WHERE nickname = $1)";
	PGConnection* pg = GDBConnectionPool->PopPG();
	pg->AddValue(pkt.nickname());

	if (false == pg->ExecuteSQL(pgCheckNicknameQuery))
	{
		Utils::LockPrint("Failed to execute SQL query:", pgCheckNicknameQuery);
		pg->Clear();
		GDBConnectionPool->Push(&pg);
		response.set_success(false);
		response.set_reason("500 DB Fail");
		session->Send(ClientPacketHandler::MakeSendBuffer(response));
		return;
	}

	if (pg->GetValue(0, 0) == "f")
	{
		response.set_success(true);
		response.set_reason("Nickname is available");
		session->Send(ClientPacketHandler::MakeSendBuffer(response));
	}
	else
	{
		response.set_success(false);
		response.set_reason("Nickname already exists");
		session->Send(ClientPacketHandler::MakeSendBuffer(response));
	}

	pg->Clear();
	GDBConnectionPool->Push(&pg);
}

void Handle_GC_CREATE_CHARACTER(const PacketSessionRef& session, const Protocol::GC_CREATE_CHARACTER& pkt)
{
	Protocol::GS_CREATE_CHARACTER response;
	Utils::LockPrint("Handle_GC_CREATE_CHARACTER");

	//트랜잭션 필요할듯?
	string pgCheckNicknameQuery = "SELECT EXISTS(SELECT 1 FROM game.characters WHERE nickname = $1)";
	PGConnection* pg = GDBConnectionPool->PopPG();
	pg->AddValue(pkt.nickname());

	if (false == pg->ExecuteSQL(pgCheckNicknameQuery))
	{
		Utils::LockPrint("Failed to execute SQL query:", pgCheckNicknameQuery);
		pg->Clear();
		GDBConnectionPool->Push(&pg);
		response.set_success(false);
		response.set_reason("500 DB Fail");
		session->Send(ClientPacketHandler::MakeSendBuffer(response));
		return;
	}
	bool nicknameExists = (pg->GetValue(0, 0) == "t");
	if (nicknameExists)
	{
		pg->Clear();
		GDBConnectionPool->Push(&pg);
		response.set_success(false);
		response.set_reason("Nickname already exists");
		session->Send(ClientPacketHandler::MakeSendBuffer(response));
		return;
	}
	pg->Clear();

	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

	string pgCreateCharacterQuery = "INSERT INTO game.characters (user_id, nickname, job_type) VALUES ($1, $2, $3)";
	pg->AddValue(::to_string(gameSession->_userId));
	pg->AddValue(pkt.nickname());
	switch (pkt.type())
	{
	case Protocol::PlayerType::PLAYER_TYPE_KNIGHT:
		pg->AddValue("1");
		break;
	case Protocol::PlayerType::PLAYER_TYPE_MAGE:
		pg->AddValue("2");
		break;
	case Protocol::PlayerType::PLAYER_TYPE_ARCHER:
		pg->AddValue("3");
		break;
	default:
		pg->Clear();
		GDBConnectionPool->Push(&pg);
		response.set_success(false);
		response.set_reason("Invalid player type");
		session->Send(ClientPacketHandler::MakeSendBuffer(response));
		return;
	}

	if (false == pg->ExecuteSQL(pgCreateCharacterQuery))
	{
		Utils::LockPrint("Failed to execute SQL query:", pgCreateCharacterQuery);
		pg->Clear();
		GDBConnectionPool->Push(&pg);
		response.set_success(false);
		response.set_reason("500 DB Fail");
		session->Send(ClientPacketHandler::MakeSendBuffer(response));
		return;
	}
	bool success = pg->IsQuerySuccessed();
	pg->Clear();
	GDBConnectionPool->Push(&pg);
	if (success)
	{
		response.set_success(true);
		response.set_reason("Successed to create character");
		session->Send(ClientPacketHandler::MakeSendBuffer(response));
	}
	else
	{
		response.set_success(false);
		response.set_reason("Failed to create character");
		session->Send(ClientPacketHandler::MakeSendBuffer(response));
	}
}

void Handle_GC_ENTER_ROOM(const PacketSessionRef& session, const Protocol::GC_ENTER_ROOM& pkt)
{
	Utils::LockPrint("Handle_GC_ENTER_ROOM");

	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);
	PlayerRef player = make_shared<Player>(gameSession);
	gameSession->_player = player;
	
	GRoom->DoAsync(&Room::Enter, player, pkt.character_index(), pkt.room_id());
}

void Handle_GC_LEAVE_GAME(const PacketSessionRef& session, const Protocol::GC_LEAVE_GAME& pkt)
{
	Utils::LockPrint("Handle_GC_LEAVE_GAME");
}

void	Handle_GC_MOVE(const PacketSessionRef& session, const Protocol::GC_MOVE& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);
	if (gameSession == nullptr)
		return;
	if (gameSession->_player == nullptr)
		return;
	PlayerRef player = gameSession->_player;
	if (player == nullptr)
		return;
	RoomRef room = player->_room.load().lock();
	
	room->DoAsync(&Room::Move, player->_info.id(), pkt);
}


void Handle_GC_LEAVE_ROOM(const PacketSessionRef& session, const Protocol::GC_LEAVE_ROOM& pkt)
{
	Utils::LockPrint("Handle_GC_LEAVE_ROOM");
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

	PlayerRef player = gameSession->_player;
	if (player == nullptr)
		return;
	RoomRef room = player->_room.load().lock();
	if (room == nullptr)
		return;
	room->DoAsync(&Room::Leave, player);
}

void	Handle_GC_CHAT(const PacketSessionRef& session, const Protocol::GC_CHAT& pkt)
{
	Utils::LockPrint("message from client:", pkt.msg());
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);
	if (gameSession->_player == nullptr)
		return;
	gameSession->_player->ChatTest(pkt.msg());
}