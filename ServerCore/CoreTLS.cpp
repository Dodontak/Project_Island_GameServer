#include "pch.h"
#include "CoreTLS.h"

thread_local int	LThreadId = 0;
thread_local JobQueue* LCurrentJobQueue = nullptr;