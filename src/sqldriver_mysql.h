#pragma once
#include <mutex>
#include "boost/lockfree/queue.hpp"
#include "sqldriver.h"
#include "mysql_driver.h" 
#include "mysql_connection.h"
#include "cppconn/driver.h"
#include "cppconn/exception.h"
#include "cppconn/resultset.h"
#include "cppconn/statement.h"

using namespace std;

class CSqlConnPool_Mysql :public CSqlConnPool
{
public:
	explicit CSqlConnPool_Mysql(const CSqlDatabasePrivate &pri);
	~CSqlConnPool_Mysql();

	void *GetConnection()override;
	void DestoryConnPool()override;		// �������ݿ����ӳ�
	void ReleaseConnection(void* conn, bool bRelease = true)override;
	bool InitConnpool(int maxSize)override;
protected:
	sql::Connection* getFreeConn();
	sql::Connection *createConnection();	// ����һ������
	void putFreeConn(sql::Connection* conn);
private:
	int											  m_maxSize;									// ���ӳص����������
	sql::Driver									* m_pDriver;
	std::atomic<int>							  m_curSize;									// ��ǰ���ӳ����Ծ��������
	boost::lockfree::queue<sql::Connection*, boost::lockfree::capacity<10>>	  m_connList;		// ���ӳص���������  boost ��������
};


class CSqlQuery_Mysql :public CSqlQuery
{
public:
	explicit CSqlQuery_Mysql(void * conn);
	~CSqlQuery_Mysql();
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
	std::string			m_sql;
	sql::Statement *	m_pStat;
	sql::ResultSet *	m_pRset;
	sql::Connection*    m_pConnection;
};