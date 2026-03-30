#include "pch.h"
#include "SendBuffer.h"
#include <cstring>
#include <iostream>

using namespace std;

SendBuffer::SendBuffer(BYTE* buffer, uint32 dataLen)
	: _writePos(0), _copyPos(0), _allocSize(dataLen)
{
	_buffer.resize(dataLen);
	memcpy(&_buffer[0], buffer, dataLen);
}

SendBuffer::SendBuffer(uint32 dataLen) : _writePos(0), _copyPos(0), _allocSize(dataLen)
{
	_buffer.resize(dataLen);
}

SendBuffer::~SendBuffer() {}

bool	SendBuffer::AppendBuffer(BYTE* buffer, uint32 dataLen)
{
	memcpy(&_buffer[_copyPos], buffer, dataLen);
	_copyPos += dataLen;
	return true;
}

bool	SendBuffer::UpdateWritePos(uint32 writeLen)
{
	_writePos += writeLen;
	if (_allocSize <= _writePos)
		return false;
	return true;
}