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
	IocpEvent() = default;
	IocpEvent(EventType eventType);
	void Init();

	void SetOwner(IocpObjectRef owner) { _owner = owner; }
	void SetEventType(EventType eventType) { _eventType = eventType; }

	IocpObjectRef GetOwner() { return _owner; }
	EventType GetEventType() { return _eventType; }
protected:
	IocpObjectRef _owner;
	EventType _eventType;
};

class AcceptEvent : public IocpEvent
{
public:
	AcceptEvent() : IocpEvent(EventType::Accept) {}
	void SetSession(SessionRef session) { _session = session; }

	void Clear();

	SessionRef GetSession() { return _session; }
	BYTE* GetAcceptBuffer() { return _acceptBuffer; }
private:
	SessionRef _session;
	BYTE _acceptBuffer[(sizeof(SOCKADDR_IN) + 16) * 2] = { 0 };
};

class RecvEvent : public IocpEvent
{
public:
	RecvEvent() : IocpEvent(EventType::Recv) {}
	RecvEvent(IocpObjectRef iocpObject) : IocpEvent(EventType::Recv) {}

	void Clear();
};

class SendEvent : public IocpEvent
{
public:
	SendEvent() : IocpEvent(EventType::Send) {}
	SendEvent(IocpObjectRef iocpObject) : IocpEvent(EventType::Send) {}
	void Clear();

	void Push(SendBufferRef sendBuffer) { _sendBuffers.push_back(sendBuffer); }
	vector<SendBufferRef> _sendBuffers;
};

class ConnectEvent : public IocpEvent
{
public:
	ConnectEvent() : IocpEvent(EventType::Connect) {}
	ConnectEvent(IocpObjectRef iocpObject) : IocpEvent(EventType::Connect) {}
};

class DisconnectEvent : public IocpEvent
{
public:
	DisconnectEvent() : IocpEvent(EventType::Disconnect) {}
	DisconnectEvent(IocpObjectRef iocpObject) : IocpEvent(EventType::Disconnect) {}
};