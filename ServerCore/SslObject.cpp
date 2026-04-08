#include "pch.h"
#include "SslObject.h"

SslObject::~SslObject()
{
	SSL_free(_ssl);
}

void SslObject::Init(SSL_CTX* ctx)
{
	ASSERT_CRASH(_ssl = SSL_new(ctx));
	ASSERT_CRASH(_rbio = BIO_new(BIO_s_mem()));
	ASSERT_CRASH(_wbio = BIO_new(BIO_s_mem()));
	SSL_set_bio(_ssl, _rbio, _wbio);
}

// rbio에 적힌 데이터를 읽어서 핸드쉐이크 하는 함수. (서버 -> 클라)
// 보낼 데이터가 있으면 wbio에 적히고, 데이터는 꺼내서 직접 보내야함.
SslStatus SslObject::Accept()
{
	int32 ret = SSL_accept(_ssl);

	if (ret == 1)
		return SslStatus::Ok;

	int32 err = SSL_get_error(_ssl, ret);
	if (err == SSL_ERROR_WANT_READ)
		return SslStatus::WantRead;
	else if (err == SSL_ERROR_WANT_WRITE)
		return SslStatus::WantWrite;
	else
		return SslStatus::Fail;
}

// rbio에 적힌 데이터를 읽어서 핸드쉐이크 하는 함수. (클라 -> 서버)
// 보낼 데이터가 있으면 wbio에 적히고, 데이터는 꺼내서 직접 보내야함.
SslStatus SslObject::Connect()
{
	int32 ret = SSL_connect(_ssl);

	if (ret == 1)
		return SslStatus::Ok;

	int32 err = SSL_get_error(_ssl, ret);
	if (err == SSL_ERROR_WANT_READ)
		return SslStatus::WantRead;
	else if (err == SSL_ERROR_WANT_WRITE)
		return SslStatus::WantWrite;
	else
		return SslStatus::Fail;
}

// rbio에 적힌 데이터를 복호화 하는 함수
SslStatus SslObject::Recv(BYTE* buffer, size_t readSize, size_t* readLen)
{
	int32 ret = SSL_read_ex(_ssl, buffer, readSize, readLen);
	if (ret == 0) // 실패
	{
		int32 err = SSL_get_error(_ssl, ret);
		if (err == SSL_ERROR_WANT_READ)//복호화 하기에 데이터 부족함
			return SslStatus::WantRead;
		else if (err == SSL_ERROR_ZERO_RETURN)
			return SslStatus::Shutdown;
		else
			return SslStatus::Fail;
	}
	return SslStatus::Ok;
}

// wbio에 데이터를 암호화 해서 쓰는 함수
SslStatus SslObject::Send(BYTE* buffer, size_t dataLen, size_t* writtenLen)
{
	int32 ret = SSL_write_ex(_ssl, buffer, dataLen, writtenLen);
	if (ret == 0) // 실패
	{
		int32 err = SSL_get_error(_ssl, ret);
		if (err == SSL_ERROR_WANT_WRITE) // wbio 공간 부족
			return SslStatus::WantWrite;
		else if (err == SSL_ERROR_ZERO_RETURN)
			return SslStatus::Shutdown;
		else
			return SslStatus::Fail;
	}
	return SslStatus::Ok;
}

// rbio에 복호화 할 수 있는 데이터가 남아있는지 확인하는 함수.
// 1 데이터 있음. 0 데이터 없음.
uint32 SslObject::HasSslPending()
{
	return SSL_has_pending(_ssl);
}

uint32 SslObject::HasRBioPending()
{
	return BIO_pending(_rbio);
}

uint32 SslObject::HasWBioPending()
{
	return BIO_pending(_wbio);
}

// bio에 암/복호화 없이 직접 읽거나 쓰는 함수들.
// 리턴값 > 0 읽거나 쓴 바이트 수, 0 -1 실패, -2 BIO 오류
uint32 SslObject::ReadRBio(BYTE* buffer, int32 readSize)
{
	return BIO_read(_rbio, buffer, readSize);
}

uint32 SslObject::WriteRBio(BYTE* buffer, int32 writeSize)
{
	return BIO_write(_rbio, buffer, writeSize);
}

uint32 SslObject::ReadWBio(BYTE* buffer, int32 readSize)
{
	return BIO_read(_wbio, buffer, readSize);
}

uint32 SslObject::WriteWBio(BYTE* buffer, int32 dataLen)
{
	return BIO_write(_wbio, buffer, dataLen);
}