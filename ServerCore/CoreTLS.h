#pragma once

#include "JobQueue.h"

extern thread_local int LThreadId;
extern thread_local JobQueue* LCurrentJobQueue;