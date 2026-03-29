#include "pch.h"
#include "IocpEvent.h"

IocpEvent::IocpEvent(EventType eventType) : _eventType(eventType)
{
	Init();
}

void IocpEvent::Init()
{
	OVERLAPPED::hEvent = 0;
	OVERLAPPED::Internal = 0;
	OVERLAPPED::InternalHigh = 0;
	OVERLAPPED::Offset = 0;
	OVERLAPPED::OffsetHigh = 0;
}

void AcceptEvent::Clear()
{
	Init();
	_owner = nullptr;
	_session = nullptr;
	memset(_acceptBuffer, 0, sizeof(_acceptBuffer));
}

void SendEvent::Clear()
{
	_owner = nullptr;
	_sendBuffers.clear();
}

void RecvEvent::Clear()
{
	_owner = nullptr;
}
