#pragma once

#include "Enum.pb.h"

namespace Protocol
{
	class ObjectInfo;
	class Position;
}

class Object : public std::enable_shared_from_this<Object>
{
public:
	Object();
	virtual ~Object();

	virtual void OnEnterGame() abstract;
	virtual void OnLeaveGame() abstract;
	virtual void Update() abstract;

	RoomRef GetRoom() { return _room.load().lock(); }
	void SetRoom(RoomRef room) { _room.store(room); }
	void ClearRoom() { _room.load().reset(); }

public:
	Protocol::ObjectInfo* GetObjectInfo() { return _objectInfo; }
	void SetObjectInfo(const Protocol::ObjectInfo& info);

	uint64 GetObjectId();
	void SetObjectId(uint64 id);

	string GetName();
	void SetName(const string& name);

	uint32 GetLevel();
	void SetLevel(uint32 level);

	uint32 GetTemplateId();
	void SetTemplateId(uint32 templateId);

	const Protocol::Position& GetPosition();
	void SetPosition(const Protocol::Position& pos);
	void SetPosition(const float x, const float y, const float z);

protected:
	atomic<weak_ptr<Room>> _room;
	Protocol::ObjectInfo* _objectInfo = nullptr;
};