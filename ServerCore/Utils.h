#pragma once

#include <mutex>

class Utils
{
public:
	static int ErrorExit(const char* errstr);
	static string GetErrorMessage(DWORD errorCode);
	static int32 GetRandNum(int32 start, int32 end);

	template<typename... Args>
	static void LockPrint(Args&&... args)
	{
		lock_guard<mutex> lock(m);
		(cout << ... << args) << endl;
	}

	private:
		static mutex m;
};

