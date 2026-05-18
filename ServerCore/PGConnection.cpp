#include "pch.h"
#include "PGConnection.h"

using namespace std;

PGConnection::~PGConnection()
{
	PQfinish(_connection);
	Clear();
}

bool	PGConnection::Connect(const char* connectionString)
{
	_connection = PQconnectdb(connectionString);
	if (PQstatus(_connection) != CONNECTION_OK)
		return false;
	return true;
}

void	PGConnection::Clear()
{
	ClearValues();
	if (_result)
	{
		PQclear(_result);
		_result = nullptr;
	}
}

void	PGConnection::AddValue(const string& val)
{
	_values.push_back(val);
}

void	PGConnection::ClearValues()
{
	_values.resize(0);
}

bool	PGConnection::ExecuteSQL(const string& sql)
{
	vector<const char*>	value_ptrs(_values.size());
	for (int i = 0; i < _values.size(); ++i)
		value_ptrs[i] = _values[i].c_str();
	_result = PQexecParams(
		_connection,
		sql.c_str(),
		_values.size(),
		NULL,
		value_ptrs.data(),
		NULL,
		NULL,
		0
	);

	ClearValues();
	ExecStatusType	ret = PQresultStatus(_result);
	if (ret != PGRES_COMMAND_OK && ret != PGRES_TUPLES_OK)//실패했다면
	{
		cout << PQerrorMessage(_connection) << endl;
		Clear();
		return false;
	}
	return true;
}

int32	PGConnection::GetRowCount()
{
	return PQntuples(_result);
}

bool	PGConnection::IsQuerySuccessed()
{
	return PQresultStatus(_result) == PGRES_COMMAND_OK;
}

string	PGConnection::GetValue(int32 row, int32 col)
{
	return PQgetvalue(_result, row, col);
}

bool	PGConnection::IsNull(int32 row, int32 col)
{
	return PQgetisnull(_result, row, col);
}