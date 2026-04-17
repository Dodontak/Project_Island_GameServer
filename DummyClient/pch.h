#pragma once

#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용을 Windows 헤더에서 제외합니다.

#ifdef _DEBUG
#pragma comment(lib, R"(Debug\ServerCore.lib)")
#else
#pragma comment(lib, R"(Release\ServerCore.lib)")
#endif

#include "CorePch.h"

#define USING_SHARED_PTR(name)	using name##Ref = std::shared_ptr<class name>;

USING_SHARED_PTR(ClientService)
USING_SHARED_PTR(ServerSession)
