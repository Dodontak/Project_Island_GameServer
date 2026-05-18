#pragma once

#include <libpq-fe.h>

class PGConnection
{
public:
	PGConnection() {}
	~PGConnection();

	bool		Connect(const char* connectionString);
	void		Clear();
	void		ClearValues();

	void		AddValue(const string& val);
	bool		ExecuteSQL(const string& sql);
	bool		IsQuerySuccessed();
	int32		GetRowCount();
	string		GetValue(int32 row, int32 col);
	bool		IsNull(int32 row, int32 col);

private:
	PGconn* _connection = nullptr;
	PGresult* _result = nullptr;
	vector<string> _values;
};