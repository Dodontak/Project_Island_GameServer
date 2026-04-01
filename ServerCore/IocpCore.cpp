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

bool IocpCore::Dispatch()
{
	DWORD numOfBytes = 0;
	ULONG_PTR completionKey;
	OVERLAPPED* overlapped = nullptr;

	bool result = GetQueuedCompletionStatus(_iocpHandle, &numOfBytes, &completionKey,
		&overlapped, INFINITE);


	if (result != 0)
	{
		IocpEvent* iocpEvent = static_cast<IocpEvent*>(overlapped);
		IocpObjectRef owner = iocpEvent->GetOwner();
		owner->Dispatch(numOfBytes, iocpEvent);
	}
	else
	{
		int32 errCode = ::WSAGetLastError();
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

