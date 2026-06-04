#include "pch.h"
#include "Room.h"
#include "Player.h"
#include "Session.h"
#include "ClientPacketHandler.h"
#include "GameSession.h"
#include "DBConnectionPool.h"
#include "Monster.h"

RoomRef GRoom;

Room::Room()
{
	static atomic<uint32> s_roomId = 0;
	_roomId = s_roomId.fetch_add(1);
}

Room::~Room() {}

void Room::Update()
{
	Utils::LockPrint("Update Room ", _roomId, " tick");
	
	for (auto Object : _objects)
	{
		Object.second->Update();
	}

	DoTimer(100, &Room::Update);
}

//플레이어(objectId)가 해당 Room에서 dest 위치로 이동했다는 사실을 Room의 다른 플레이어들에게 알림
void Room::Move(uint64 objectId, const Protocol::Position& pos, float speed)
{
	Protocol::GS_MOVE movePkt;

	if (_players.find(objectId) == _players.end() && _objects.find(objectId) == _objects.end())
		return;

	//TODO 유효한 위치인지 확인

	movePkt.set_object_id(objectId);
	movePkt.mutable_dest()->CopyFrom(pos);
	movePkt.set_speed(speed);
	Broadcast(ClientPacketHandler::MakeSendBuffer(movePkt));
}

void Room::Enter(PlayerRef player, int32 characterIndex, int32 roomId)
{
	//TODO DB쿼리때문에 렉 심하면 비동기로 바꿔야할 듯
	Protocol::GS_ENTER_ROOM response;
	GameSessionRef gameSession = player->GetOwner();
	if (gameSession == nullptr)//TODO 적절한 처리
		return;

	//캐릭터 데이터 가져오기
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
		gameSession->Send(ClientPacketHandler::MakeSendBuffer(response));
		return;
	}

	int32 rowCount = pg->GetRowCount();
	if (rowCount < characterIndex)
	{
		pg->Clear();
		GDBConnectionPool->Push(&pg);
		response.set_success(false);
		response.set_reason("Character not found");
		gameSession->Send(ClientPacketHandler::MakeSendBuffer(response));
		return;
	}

	//데이터를 저장
	Protocol::ObjectInfo characterInfo;
	Protocol::Position position;
	characterInfo.set_object_id(Utils::GetObjectId());
	characterInfo.set_name(pg->GetValue(characterIndex, 0));
	characterInfo.set_level(stoi(pg->GetValue(characterIndex, 1)));
	uint32 templateId = Utils::GetTemplateId(0, Protocol::ObjectType::OBJECT_TYPE_CREATURE,
		Protocol::CreatureType::CREATURE_TYPE_PLAYER, stoi(pg->GetValue(characterIndex, 2)));
	characterInfo.set_template_id(templateId);

	position.set_x(Utils::GetRandom(0, 1000));
	position.set_y(Utils::GetRandom(0, 1000));
	position.set_z(500);
	position.set_pitch(0);
	position.set_yaw(0);
	position.set_roll(0);
	characterInfo.mutable_pos()->CopyFrom(position);

	pg->Clear();
	GDBConnectionPool->Push(&pg);

	//입장
	player->SetObjectInfo(characterInfo);
	_players.insert(make_pair(player->GetObjectId(), player));
	player->SetRoom(static_pointer_cast<Room>(shared_from_this()));

	//입장을 유저에게 알림
	response.mutable_character_info()->CopyFrom(characterInfo);
	response.set_success(true);
	gameSession->Send(ClientPacketHandler::MakeSendBuffer(response));

	//해당 플레이어의 입장을 다른 플레이어들에게 알림
	for (auto& data : _players)
	{
		int32 userId = data.first;
		PlayerRef& otherPlayer = data.second;
		if (userId == player->GetObjectId())
			continue;
		Protocol::GS_SPAWN pkt;
		Protocol::ObjectInfo* otherPlayerInfo = pkt.add_objects();
		otherPlayerInfo->CopyFrom(characterInfo);

		GameSessionRef otherSession = otherPlayer->GetOwner();
		if (otherSession == nullptr)
			continue;
		otherSession->Send(ClientPacketHandler::MakeSendBuffer(pkt));
	}

	//TODO 아래 두가지 합쳐도 될듯?
	//해당 플레이어의 클라에 다른 플레이어의 존재를 알림
	{
		Protocol::GS_SPAWN pkt;
		for (auto& data : _players)
		{
			PlayerRef& otherPlayer = data.second;
			if (otherPlayer->GetObjectId() == player->GetObjectId())
				continue;
			Protocol::ObjectInfo* otherPlayerInfo = pkt.add_objects();
			otherPlayerInfo->CopyFrom(*otherPlayer->GetObjectInfo());
		}
		if (pkt.objects_size() > 0)
			gameSession->Send(ClientPacketHandler::MakeSendBuffer(pkt));
	}

	//해당 플레이어의 클라에 플레이어 외의 다른 오브젝트의 존재를 알림
	{
		Protocol::GS_SPAWN pkt;
		for (auto& data : _objects)
		{
			ObjectRef& otherObject = data.second;
			Protocol::ObjectInfo* otherObjectInfo = pkt.add_objects();
			otherObjectInfo->CopyFrom(*otherObject->GetObjectInfo());
		}
		if (pkt.objects_size() > 0)
			gameSession->Send(ClientPacketHandler::MakeSendBuffer(pkt));
	}
}

void Room::Leave(PlayerRef player)
{
	if (player == nullptr)
		return;
	const uint64 objectId = player->GetObjectId();

	_players.erase(objectId);
	player->ClearRoom();

	//퇴장 사실을 퇴장하는 플레이어에게 알림
	{
		Protocol::GS_LEAVE_ROOM pkt;

		if (GameSessionRef gameSession = player->GetOwner())
			gameSession->Send(ClientPacketHandler::MakeSendBuffer(pkt));
	}

	//퇴장 사실을 다른 플레이어들에게 알림
	{
		Protocol::GS_DESPAWN pkt;
		pkt.add_object_ids(objectId);

		Broadcast(ClientPacketHandler::MakeSendBuffer(pkt));
	}
}

void Room::AddObject(ObjectRef object)
{
	Protocol::GS_SPAWN pkt;
	_objects.insert(make_pair(object->GetObjectId(), object));
	object->SetRoom(static_pointer_cast<Room>(shared_from_this()));

	Protocol::ObjectInfo* obj = pkt.add_objects();
	obj->CopyFrom(*object->GetObjectInfo());
	Broadcast(ClientPacketHandler::MakeSendBuffer(pkt));
}

void Room::Broadcast(SendBufferRef sendBuffer)
{
	for (auto& player : _players)
	{
		if (auto owner = player.second->GetOwner())
		{
			owner->Send(sendBuffer);
		}
	}
}
