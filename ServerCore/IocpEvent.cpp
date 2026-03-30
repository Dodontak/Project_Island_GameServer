#include "pch.h"
#include "IocpEvent.h"
#include "SendBuffer.h"

/*----------------------------------------------------------------------------*\
|                                 IocpEvent                                    |
\*----------------------------------------------------------------------------*/
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

/*----------------------------------------------------------------------------*\
|                                AcceptEvent                                   |
\*----------------------------------------------------------------------------*/
void AcceptEvent::Clear()
{
	Init();
	_owner = nullptr;
	_session = nullptr;
	memset(_acceptBuffer, 0, sizeof(_acceptBuffer));
}

/*----------------------------------------------------------------------------*\
|                                 SendEvent                                    |
\*----------------------------------------------------------------------------*/
void SendEvent::Clear()
{
	_owner = nullptr;
	_sendBuffers.clear();
	_wantSendBytes = 0;
}

void SendEvent::PushFront(SendBufferRef sendBuffer)
{
	_sendBuffers.push_front(sendBuffer);
	_wantSendBytes += sendBuffer->GetDataLen();
}

void SendEvent::PushBack(SendBufferRef sendBuffer)
{
	_sendBuffers.push_back(sendBuffer);
	_wantSendBytes += sendBuffer->GetDataLen();
}

/*----------------------------------------------------------------------------*\
|                                 RecvEvent                                    |
\*----------------------------------------------------------------------------*/
void RecvEvent::Clear()
{
	_owner = nullptr;
}
