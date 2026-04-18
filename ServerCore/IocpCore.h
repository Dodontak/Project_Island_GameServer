#pragma once

class IocpEvent;

class IocpObject : public enable_shared_from_this<IocpObject>
{
public:
	virtual HANDLE GetHandle() abstract;
	virtual void Dispatch(int32 numOfBytes, IocpEvent* event) abstract;
};

class IocpCore
{
public:
	IocpCore();
	~IocpCore();

	bool Dispatch(uint32 timeoutMs);

	bool RegisterHandle(IocpObjectRef iocpObject);
private:
	HANDLE _iocpHandle;
};