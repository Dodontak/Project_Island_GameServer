#include "pch.h"
#include "RecvBuffer.h"
#include <cstring>

RecvBuffer::RecvBuffer(uint32 bufferSize) : _bufferSize(bufferSize)
{
	_capacity = bufferSize * BUFFER_COUNT;
	_buffer.resize(_capacity);
}

RecvBuffer::~RecvBuffer()
{

}

void	RecvBuffer::Clean()
{
	int	dataSize = DataSize();
	if (dataSize == 0)
	{
		_readPos = _writePos = 0;
	}
	else
	{
		if (FreeSize() < _bufferSize)
		{
			memcpy(&_buffer[0], &_buffer[_readPos], dataSize);
			_readPos = 0;
			_writePos = dataSize;
		}
	}
}

bool	RecvBuffer::OnRead(uint32 numOfBytes)
{
	if (numOfBytes > DataSize())
		return false;
	_readPos += numOfBytes;
	return true;
}

bool	RecvBuffer::OnWrite(uint32 numOfBytes)
{
	if (numOfBytes > FreeSize())
		return false;
	_writePos += numOfBytes;
	return true;
}