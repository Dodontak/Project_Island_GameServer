#pragma once

#include "Types.h"
#include <vector>

class RecvBuffer
{
	enum { BUFFER_COUNT = 10 };
public:
	RecvBuffer(uint32 bufferSize);
	~RecvBuffer();

	bool	OnRead(uint32 numOfBytes);
	bool	OnWrite(uint32 numOfBytes);

	void	Clean();
	BYTE*	ReadPos() { return &_buffer[_readPos]; }
	BYTE*	WritePos() { return &_buffer[_writePos]; }
	uint32	DataSize() { return _writePos - _readPos; }
	uint32	FreeSize() { return _capacity - _writePos; }

private:
	uint32			_capacity = 0;
	uint32			_bufferSize = 0;
	uint32			_readPos = 0;
	uint32			_writePos = 0;
	vector<BYTE>	_buffer;
};