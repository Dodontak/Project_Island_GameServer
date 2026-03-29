#pragma once

enum class EventType : uint8
{
	Connect,
	Disconnect,
	Accept,
	Recv,
	Send
};

class IocpEvent : public OVERLAPPED
{
public:
	IocpEvent(IocpObjectRef iocpObject, EventType eventType);
	EventType GetEventType() { return _eventType; }
	IocpObjectRef GetOwner() { return _owner; }
private:
	void Init();
	IocpObjectRef _owner;
	EventType _eventType;
};

class AcceptEvent : public IocpEvent
{
public:
	AcceptEvent(IocpObjectRef iocpObject) : IocpEvent(iocpObject, EventType::Accept) {}
	void SetSession(SessionRef session) { _session = session; }
	SessionRef GetSession() { return _session; }
	BYTE* GetAcceptBuffer() { return _acceptBuffer; }
private:
	SessionRef _session;
	BYTE _acceptBuffer[(sizeof(SOCKADDR_IN) + 16) * 2] = {0};
};

class RecvEvent : public IocpEvent
{
public:
	RecvEvent(IocpObjectRef iocpObject) : IocpEvent(iocpObject, EventType::Recv) {}
};