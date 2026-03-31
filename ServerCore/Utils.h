#pragma once


class Utils
{
public:
	static int HandleError(const char* errstr);
	static string GetErrorMessage(DWORD errorCode);
};

