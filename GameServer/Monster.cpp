#include "pch.h"
#include "Monster.h"
#include "Room.h"
#include "Struct.pb.h"

Monster::Monster(uint8 type)
{
	_objectInfo->set_object_id(Utils::GetObjectId());
	uint32 templateId = Utils::GetTemplateId(0, Protocol::ObjectType::OBJECT_TYPE_CREATURE,
		Protocol::CreatureType::CREATURE_TYPE_MONSTER, type);
	_objectInfo->set_template_id(templateId);
	_objectInfo->set_level(1);
	switch (type)
	{
	case Protocol::MonsterType::MONSTER_TYPE_SKELETON:
		_maxHp = 100;
		_damage = 10;
		_speed = 300.f;
		_objectInfo->set_name("Skeleton");
		break;
	case Protocol::MonsterType::MONSTER_TYPE_WEREWOLF:
		_maxHp = 200;
		_damage = 20;
		_speed = 350.f;
		_objectInfo->set_name("Werewolf");
		break;
	case Protocol::MonsterType::MONSTER_TYPE_STONEGOLEM:
		_maxHp = 500;
		_damage = 30;
		_speed = 200.f;
		_objectInfo->set_name("Stone Golem");
		break;
	default:
		_maxHp = 100;
		_damage = 10;
		_speed = 300.f;
		_objectInfo->set_name("Unknown Monster");
		break;
	}
	_hp = _maxHp;
}

Monster::~Monster()
{}

void Monster::OnEnterGame()
{}

void Monster::OnLeaveGame()
{}

void Monster::Update()
{
	RoomRef room = GetRoom();
	if (room == nullptr)
		return;

	Protocol::Position* pos = _objectInfo->mutable_pos();
	float x = pos->x();
	float y = pos->y();


	if (x < 800 && y > 800)
	{
		//Right
		x += _speed * 0.1f;
		pos->set_yaw(0);
	}
	else if (x > 800 && y > -800)
	{
		//Down
		y -= _speed * 0.1f;
		pos->set_yaw(-90);
	}
	else if (x > -800 && y < -800)
	{
		//Left
		x -= _speed * 0.1f;
		pos->set_yaw(180);
	}
	else if (x < -800 && y < 800)
	{
		//Up
		y += _speed * 0.1f;
		pos->set_yaw(90);
	}
	else
	{
		//Down
		y -= _speed * 0.1f;
		pos->set_yaw(-90);
	}
	pos->set_x(x);
	pos->set_y(y);

	room->DoAsync(&Room::Move, GetObjectId(), *pos, _speed);
}
