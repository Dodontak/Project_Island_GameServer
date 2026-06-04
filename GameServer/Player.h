#pragma once
#include "Struct.pb.h"
#include "Creature.h"

class Room;

class Player : public Creature
{
public:
	Player(GameSessionRef session);
	virtual ~Player() override;

	virtual void OnEnterGame() override;
	virtual void OnLeaveGame() override;
	virtual void Update() override;

public:
	void ChatTest(const string& msg);

	shared_ptr<GameSession> GetOwner() { return _owner.lock(); }

protected:
	weak_ptr<GameSession> _owner;
};
