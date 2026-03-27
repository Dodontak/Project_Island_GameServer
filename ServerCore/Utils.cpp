#include "pch.h"
#include "Utils.h"

int Utils::HandleError(const char* errstr)
{
	cerr << errstr << endl;
	exit(1);
	return 1;
}

