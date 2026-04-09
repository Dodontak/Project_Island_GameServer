#pragma once

#include <hiredis/hiredis.h>
#include <string>

class RedisConnection
{
public:
	RedisConnection() {}
	~RedisConnection();

	bool	Connect(const char* ip, int32 port);
	void	Clear();

	string	GetStr() { return _reply->str; }
	size_t	GetStrLen() { return _reply->len; }
	int64	GetInt() { return _reply->integer; }
	bool	IsNull() { return _reply->type == REDIS_REPLY_NIL; }
	//elements는 어떻게 할까 흠. 일단 인증서버에선 안씀.

	template<typename... Args>
	bool	Execute(const string& query, Args&&... args)
	{
		_reply = (redisReply*)redisCommand(_connection, query.c_str(), forward<Args>(args)...);
		if (IsReplyError())
		{
			Clear();
			return false;
		}
		return true;
	}

private:
	bool	IsReplyError();

private:
	redisContext* _connection = nullptr;
	redisReply* _reply = nullptr;
};