#include "pch.h"
#include <windows.h>
#include "Utils.h"

int Utils::HandleError(const char* errstr)
{
	cerr << errstr << endl;
	exit(1);
	return 1;
}

string Utils::GetErrorMessage(DWORD errorCode)
{
    LPSTR buffer = nullptr;

    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        errorCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        reinterpret_cast<LPSTR>(&buffer),
        0,
        nullptr
    );

    std::string message(buffer ? buffer : "Unknown error");
    LocalFree(buffer);
    return message;
}