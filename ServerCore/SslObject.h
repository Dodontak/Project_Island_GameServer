#pragma once

#include <openssl/ssl.h>

enum SslStatus
{
	Ok,
	WantRead,
	WantWrite,
	Shutdown,
	Fail
};

class SslObject
{
public:
	SslObject() = default;
	~SslObject();

	void Init(SSL_CTX* ctx);

	SslStatus Accept();
	SslStatus Connect();
	SslStatus Read(BYTE* buffer, size_t readSize, size_t* readLen);
	SslStatus Write(BYTE* buffer, size_t dataLen, size_t* writtenLen);

	uint32 GetRBioPendingSize();
	uint32 GetWBioPendingSize();

	uint32 ReadRBio(BYTE* buffer, int32 readSize);
	uint32 WriteRBio(BYTE* buffer, int32 readSize);

	uint32 WriteWBio(BYTE* buffer, int32 dataLen);
	uint32 ReadWBio(BYTE* buffer, int32 readSize);


private:
	SSL* _ssl = nullptr;
	BIO* _rbio = nullptr;
	BIO* _wbio = nullptr;
};

