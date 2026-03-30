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
	BYTE*	GetCopyBuffer() { return &_buffer[_copyPos]; }

public: //use at write
	BYTE*	GetBuffer() { return &_buffer[_writePos]; }
	BYTE*	GetPosPtr(uint32 pos) { return &_buffer[pos]; }
	uint32	GetDataLen() { return _allocSize - _writePos; }
	bool	UpdateWritePos(uint32 writeLen);

private:
	uint32			_writePos;
	uint32			_copyPos;
	uint32			_allocSize;
	vector<BYTE>	_buffer;
};