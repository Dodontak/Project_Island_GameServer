#include "pch.h"
#include "IocpCore.h"
#include "IocpEvent.h"

IocpCore::IocpCore()
{
	_iocpHandle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	ASSERT_CRASH(_iocpHandle != INVALID_HANDLE_VALUE);
}

IocpCore::~IocpCore()
{
	::CloseHandle(_iocpHandle);
}

bool IocpCore::Dispatch()
{
	DWORD numOfBytes = 0;
	ULONG_PTR completionKey;
	IocpEvent* event = nullptr;

	bool result = GetQueuedCompletionStatus(_iocpHandle, &numOfBytes, &completionKey,
		reinterpret_cast<LPOVERLAPPED*>(&event), INFINITE);

	if (result != 0) // 정상적으로 이벤트가 발생한 경우
	{
		IocpObjectRef owner = event->GetOwner();
		owner->Dispatch(numOfBytes, event);
	}
	else // 오류가 발생한 경우
	{

	}
	return true;
}

bool IocpCore::RegisterHandle(IocpObjectRef iocpObject)
{
	if (CreateIoCompletionPort(iocpObject->GetHandle(), _iocpHandle, 0, 0) == nullptr)
		return false;
	return true;
}

