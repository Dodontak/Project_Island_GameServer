#include "pch.h"
#include "SendBuffer.h"
#include <cstring>
#include <iostream>

using namespace std;

SendBuffer::SendBuffer(BYTE* buffer, uint32 dataLen)
	: _writePos(dataLen), _allocSize(dataLen)
{
	_buffer.resize(dataLen);
	memcpy(&_buffer[0], buffer, dataLen);
}

SendBuffer::SendBuffer(uint32 dataLen) : _writePos(0), _allocSize(dataLen)
{
	_buffer.resize(dataLen);
}

SendBuffer::~SendBuffer() {}

bool	SendBuffer::AppendBuffer(BYTE* buffer, uint32 dataLen)
{
	memcpy(&_buffer[_writePos], buffer, dataLen);
	_writePos += dataLen;
	return true;
}

bool SendBuffer::OnWrite(uint32 dataLen)
{
	if (dataLen > GetFreeSize())
		return false;
	_writePos += dataLen;
	return true;
}
