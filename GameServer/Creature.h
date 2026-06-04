#pragma once

#include "Object.h"

namespace Protocol
{
	class CreatureInfo;
}

class Creature : public Object
{
public:
	Creature();
	virtual ~Creature() override;

	virtual void OnEnterGame() abstract;
	virtual void OnLeaveGame() abstract;
	virtual void Update() abstract;
protected:
};

