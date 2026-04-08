#pragma once

enum class EventType : uint8
{
	Connect,
	Disconnect,
	Accept,
	Recv,
	Send
};

/*----------------------------------------------------------------------------*\
|                                 IocpEvent                                    |
\*----------------------------------------------------------------------------*/
class IocpEvent : public OVERLAPPED
{
public:
	IocpEvent() = default;
	IocpEvent(EventType eventType);
	void Init();

	void SetOwner(IocpObjectRef owner) { _owner = owner; }
	void SetEventType(EventType eventType) { _eventType = eventType; }

	IocpObjectRef GetOwner() { return _owner; }
	EventType GetEventType() { return _eventType; }

	virtual void Clear();
protected:
	IocpObjectRef _owner;
	EventType _eventType;
};

/*----------------------------------------------------------------------------*\
|                                AcceptEvent                                   |
\*----------------------------------------------------------------------------*/
class AcceptEvent : public IocpEvent
{
public:
	AcceptEvent() : IocpEvent(EventType::Accept) {}
	void SetSession(SessionRef session) { _session = session; }

	virtual void Clear() override;

	SessionRef GetSession() { return _session; }
	BYTE* GetAcceptBuffer() { return _acceptBuffer; }
private:
	SessionRef _session;
	BYTE _acceptBuffer[(sizeof(SOCKADDR_IN) + 16) * 2] = { 0 };
};

/*----------------------------------------------------------------------------*\
|                                 RecvEvent                                    |
\*----------------------------------------------------------------------------*/
class RecvEvent : public IocpEvent
{
public:
	RecvEvent() : IocpEvent(EventType::Recv) {}
	RecvEvent(IocpObjectRef iocpObject) : IocpEvent(EventType::Recv) {}
};

/*----------------------------------------------------------------------------*\
|                                 SendEvent                                    |
\*----------------------------------------------------------------------------*/
class SendEvent : public IocpEvent
{
public:
	SendEvent() : IocpEvent(EventType::Send), _wantSendBytes(0) {}
	SendEvent(IocpObjectRef iocpObject) : IocpEvent(EventType::Send), _wantSendBytes(0) {}
	virtual void Clear() override;

	void PushBack(SendBufferRef sendBuffer);
	void PushFront(SendBufferRef sendBuffer);
	deque<SendBufferRef>& GetSendBuffers() { return _sendBuffers; }
	int32 GetWantSendBytes() { return _wantSendBytes; }

private:
	deque<SendBufferRef> _sendBuffers;
	int32 _wantSendBytes;
};

/*----------------------------------------------------------------------------*\
|                               ConnectEvent                                   |
\*----------------------------------------------------------------------------*/
class ConnectEvent : public IocpEvent
{
public:
	ConnectEvent() : IocpEvent(EventType::Connect) {}
	ConnectEvent(IocpObjectRef iocpObject) : IocpEvent(EventType::Connect) {}
};

/*----------------------------------------------------------------------------*\
|                              DisconnectEvent                                 |
\*----------------------------------------------------------------------------*/
class DisconnectEvent : public IocpEvent
{
public:
	DisconnectEvent() : IocpEvent(EventType::Disconnect) {}
	DisconnectEvent(IocpObjectRef iocpObject) : IocpEvent(EventType::Disconnect) {}
};