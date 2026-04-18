#include "pch.h"
#include "IocpCore.h"
#include "IocpEvent.h"

IocpCore::IocpCore()
{
	_iocpHandle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	ASSERT_CRASH(_iocpHandle != INVALID_HANDLE_VALUE);
	cout << "IocpCore constucted" << endl;
}

IocpCore::~IocpCore()
{
	::CloseHandle(_iocpHandle);
}

bool IocpCore::Dispatch(uint32 timeoutMs)
{
	DWORD numOfBytes = 0;
	ULONG_PTR completionKey;
	OVERLAPPED* overlapped = nullptr;

	bool result = GetQueuedCompletionStatus(_iocpHandle, &numOfBytes, &completionKey,
		&overlapped, timeoutMs);


	if (result != 0)
	{
		IocpEvent* iocpEvent = static_cast<IocpEvent*>(overlapped);
		IocpObjectRef owner = iocpEvent->GetOwner();
		owner->Dispatch(numOfBytes, iocpEvent);
	}
	else
	{
		int32 errCode = ::WSAGetLastError();
		if (errCode == WAIT_TIMEOUT)
			return false;
		string errStr = Utils::GetErrorMessage(errCode);
		errStr.pop_back();
		Utils::LockPrint(errStr);

		IocpEvent* iocpEvent = static_cast<IocpEvent*>(overlapped);
		IocpObjectRef owner = iocpEvent->GetOwner();
		owner->Dispatch(numOfBytes, iocpEvent);
	}

	return true;
}

bool IocpCore::RegisterHandle(IocpObjectRef iocpObject)
{
	if (CreateIoCompletionPort(iocpObject->GetHandle(), _iocpHandle, 0, 0) == nullptr)
		return false;
	return true;
}

