#pragma once

#include "Creature.h"
#include "Struct.pb.h"

class Monster : public Creature
{
public:
	Monster(uint8 type);
	virtual ~Monster() override;

	virtual void OnEnterGame() override;
	virtual void OnLeaveGame() override;
	virtual void Update() override;

protected:
	int32 _maxHp;
	int32 _hp;
	int32 _damage;
	float _speed;
};

