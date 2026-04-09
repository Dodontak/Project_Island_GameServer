#pragma once

#include <mutex>

class Utils
{
public:
	static int ErrorExit(const char* errstr);
	static string GetErrorMessage(DWORD errorCode);

	template<typename... Args>
	static void LockPrint(Args&&... args)
	{
		lock_guard<mutex> lock(m);
		(cout << ... << args) << endl;
	}

	private:
		static mutex m;
};

