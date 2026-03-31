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
	OVERLAPPED* overlapped = nullptr;

	bool result = GetQueuedCompletionStatus(_iocpHandle, &numOfBytes, &completionKey,
		&overlapped, INFINITE);

	if (result != 0) // 정상적으로 이벤트가 발생한 경우
	{
		IocpEvent* iocpEvent = static_cast<IocpEvent*>(overlapped);
		IocpObjectRef owner = iocpEvent->GetOwner();
		owner->Dispatch(numOfBytes, iocpEvent);
	}
	else // TODO 오류가 발생한 경우
	{
		cerr << "error" << endl;
	}
	return true;
}

bool IocpCore::RegisterHandle(IocpObjectRef iocpObject)
{
	if (CreateIoCompletionPort(iocpObject->GetHandle(), _iocpHandle, 0, 0) == nullptr)
		return false;
	return true;
}

