#pragma once

#include "Types.h"
#include <vector>

class SendBuffer : public enable_shared_from_this<SendBuffer>
{
public:
	SendBuffer(BYTE* buffer, uint32 dataLen);
	SendBuffer(uint32 dataLen);//for AppendBuffer
	~SendBuffer();

public: //use at fill buffer
	bool	AppendBuffer(BYTE* buffer, uint32 dataLen);
	BYTE* GetCopyBuffer() { return &_buffer[_writePos]; }

public: //use at write
	BYTE* GetBuffer() { return &_buffer[0]; }
	BYTE* GetPosPtr(uint32 pos) { return &_buffer[pos]; }
	uint32 GetFreeSize() { return _allocSize - _writePos; }
	uint32 GetDataLen() { return _writePos; }
	bool OnWrite(uint32 dataLen);

private:
	uint32			_writePos;
	uint32			_allocSize;
	vector<BYTE>	_buffer;
};