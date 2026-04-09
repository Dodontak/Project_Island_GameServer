#pragma once

#include "PGConnection.h"
#include "RedisConnection.h"

class DBConnectionPool
{
public:
	DBConnectionPool() {}
	~DBConnectionPool() {}

	bool	Init(int32 maxRedis, const char* redisIp, int32 redisPort,
		int32 maxPostgres, const char* pgConString);

	void	Push(PGConnection** conn);
	void	Push(RedisConnection** conn);

	PGConnection* PopPG();
	RedisConnection* PopRedis();
private:
	mutex	_mPostgres;
	int32	_maxPostgres;
	string	_pgConString;

	mutex	_mRedis;
	int32	_maxRedis;
	string	_redisIp;
	int32	_redisPort;

	vector<PGConnection*>		_postgresConnections;
	vector<RedisConnection*>	_redisConnections;
};