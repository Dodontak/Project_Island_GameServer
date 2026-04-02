#pragma once

#include <mutex>

class Utils
{
public:
	static int HandleError(const char* errstr);
	static string GetErrorMessage(DWORD errorCode);

	static SendBufferRef MakeChatSendBuffer(uint16 packetId, BYTE* buffer, uint32 size);

	template<typename... Args>
	static void LockPrint(Args&&... args)
	{
		lock_guard<mutex> lock(m);
		(cout << ... << args) << endl;
	}

	private:
		static mutex m;
};

