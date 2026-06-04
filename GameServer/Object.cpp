#include "pch.h"
#include "Object.h"
#include "Struct.pb.h"

Object::Object()
{
	_objectInfo = new Protocol::ObjectInfo();
}

Object::~Object()
{
	delete _objectInfo;
}

void Object::SetObjectInfo(const Protocol::ObjectInfo& info)
{
	_objectInfo->CopyFrom(info);
}

uint64 Object::GetObjectId()
{
	return _objectInfo->object_id();
}

void Object::SetObjectId(uint64 id)
{
	_objectInfo->set_object_id(id);
}

const Protocol::Position& Object::GetPosition()
{
	return _objectInfo->pos();
}

void Object::SetPosition(const Protocol::Position& pos)
{
	_objectInfo->mutable_pos()->CopyFrom(pos);
}

void Object::SetPosition(const float x, const float y, const float z)
{
	_objectInfo->mutable_pos()->set_x(x);
	_objectInfo->mutable_pos()->set_y(y);
	_objectInfo->mutable_pos()->set_z(z);
}

string Object::GetName()
{
	return _objectInfo->name();
}

void Object::SetName(const string& name)
{
	_objectInfo->set_name(name);
}

uint32 Object::GetLevel()
{
	return _objectInfo->level();
}

void Object::SetLevel(uint32 level)
{
	_objectInfo->set_level(level);
}

uint32 Object::GetTemplateId()
{
	return _objectInfo->template_id();
}

void Object::SetTemplateId(uint32 templateId)
{
	_objectInfo->set_template_id(templateId);
}
