#pragma once
#include "sqldriver.h"
#include "occi.h"

using namespace std;
using namespace oracle::occi;

class CSqlConnPool_Oracle:public CSqlConnPool
{
public:
	explicit CSqlConnPool_Oracle(const CSqlDatabasePrivate &pri);
	~CSqlConnPool_Oracle();

	void *GetConnection()override;
	void DestoryConnPool()override;		// 销毁数据库连接池
	void ReleaseConnection(void* conn, bool bRelease = true)override;
	bool InitConnpool(int maxSize)override;
	
private:
	Environment  * m_env;
	StatelessConnectionPool * m_connPool;
};


class CSqlQuery_Oracle :public CSqlQuery
{
public:
	explicit CSqlQuery_Oracle(void * conn);
	~CSqlQuery_Oracle();
	void * release()override;
	void setAutoCommit(bool autoCommit = true)override;
	bool prepare(const std::string &sql = "")override;
	bool executeUpdate(const std::string &sql = "")override;
	bool executeQuery(const std::string &sql = "")override;
	bool execute(const std::string &sql = "")override;
	bool exec()override;
	bool next()override;

	CVariant value(const int &index, const CVariant &defvalue)override;
	CVariant value(const std::string& name, const CVariant &defvalue)override;

	void bindValue(const int &nIndex, const int &x)override;
	void bindValue(const int &nIndex, const double &x)override;
	void bindValue(const int &nIndex, const float &x)override;
	void bindValue(const int &nIndex, const unsigned long &x)override;
	void bindValue(const int &nIndex, const std::string &x)override;

private:	
	oracle::occi::Statement *	m_pStat;
	oracle::occi::ResultSet *	m_pRset;
	oracle::occi::Connection*   m_pConnection;
};