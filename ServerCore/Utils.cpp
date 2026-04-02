#include "pch.h"
#include <windows.h>
#include "Utils.h"
#include "Session.h"

mutex Utils::m;

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


SendBufferRef Utils::MakeChatSendBuffer(uint16 packetId, BYTE* dataBuffer, uint32 dataSize)
{
	PacketHeader header;
	uint16 headerSize = sizeof(PacketHeader);
	header.id = packetId; //S_CHAT 뭐 이런 패킷 ID 나중에 protobuf, 패킷핸들러 만들때 제대로 구현
	header.size = dataSize + headerSize; //지금은 그냥 그대로 보내주는거니까 그대로
	SendBufferRef sendBuffer = make_shared<SendBuffer>(header.size);
	sendBuffer->AppendBuffer((BYTE*)&header, headerSize);
	sendBuffer->AppendBuffer(dataBuffer, dataSize);
	return sendBuffer;
}