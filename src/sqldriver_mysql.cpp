#include "sqldriver_mysql.h"
#include "log.h"

using namespace std;
CSqlConnPool_Mysql::CSqlConnPool_Mysql(const CSqlDatabasePrivate &pri) :CSqlConnPool(pri), m_curSize(0), m_pDriver(nullptr)
{
	m_maxSize = 0;
}

CSqlConnPool_Mysql::~CSqlConnPool_Mysql()
{
	DestoryConnPool();
}

void CSqlConnPool_Mysql::DestoryConnPool()
{
	try
	{
		sql::Connection * pConn;
		while (m_connList.pop(pConn))
		{
			pConn->close();
			delete pConn;
		}
		m_curSize = 0;
	}
	catch (sql::SQLException &ex)
	{
		LERROR("release connection error: {}", ex.what());
		m_lasterror = ex.what();
	}
}

sql::Connection * CSqlConnPool_Mysql::createConnection()
{
	LDEBUG("createConnection {},{},{}", m_private.source.c_str(), m_private.user.c_str(), m_private.password.c_str());
	sql::Connection *conn = m_pDriver->connect(m_private.source.c_str(), m_private.user.c_str(), m_private.password.c_str()); //建立连接
	conn->setSchema(m_private.database.c_str());
	m_curSize++;
	return conn;
}

void CSqlConnPool_Mysql::putFreeConn(sql::Connection* conn)
{
	m_connList.push(conn);
}

sql::Connection* CSqlConnPool_Mysql::getFreeConn()
{
	sql::Connection* conn = nullptr;
	m_connList.pop(conn);
	return conn;
}

bool CSqlConnPool_Mysql::InitConnpool(int maxSize)
{
	try
	{
		LDEBUG("InitConnpool {}", maxSize);
		m_maxSize = maxSize;
		m_pDriver = get_driver_instance();
		for (int i = 0; i < m_maxSize; ++i)
		{
			sql::Connection *conn = createConnection();
			putFreeConn(conn);
		}
	}
	catch (sql::SQLException &ex)
	{
		LERROR("create connPool error: {}", ex.what());
		m_lasterror = ex.what();
		return false;
	}
	return true;
}

void * CSqlConnPool_Mysql::GetConnection()
{
	sql::Connection * conn = getFreeConn();
	try
	{
		if (conn == NULL)
		{
			if (m_curSize < m_maxSize)
			{
				conn = createConnection();
			}
		}
		else if (conn->isClosed())
		{
			delete conn;
			m_curSize--;
			conn = createConnection();
		}
	}
	catch (sql::SQLException &ex)
	{
		LERROR("GetConnection error: {}", ex.what());
		m_lasterror = ex.what();
	}
	return conn;
}

void CSqlConnPool_Mysql::ReleaseConnection(void *conn, bool bRelease)
{
	try
	{
		if (bRelease)
		{
			putFreeConn((sql::Connection *)conn);
		}
		else if (conn)
		{
			sql::Connection * pConn = static_cast<sql::Connection *>(conn);
			pConn->close();
			delete pConn;
			m_curSize--;
		}
	}
	catch (sql::SQLException &ex)
	{
		LERROR("ReleaseConnection error: {}", ex.what());
		m_lasterror = ex.what();
	}
}


//************************************ CSqlQuery_Oracle *************************************//
CSqlQuery_Mysql::CSqlQuery_Mysql(void * conn) :m_pConnection(nullptr), m_pStat(nullptr), m_pRset(nullptr)
{
	try
	{
		m_nIndex = 0;
		m_pConnection = static_cast<sql::Connection*>(conn);
		m_pStat = m_pConnection->createStatement();
	}
	catch (sql::SQLException &ex)
	{
		LERROR("init oracle query failed: {}", ex.what());
		m_lasterror = ex.what();
	}
}

CSqlQuery_Mysql::~CSqlQuery_Mysql()
{

}

void * CSqlQuery_Mysql::release()
{
	try
	{
		if (m_pConnection)
		{
			if (m_pRset)
			{
				m_pRset->close();
				delete m_pRset;
			}
			if (m_pStat)
			{
				m_pStat->close();
				delete m_pStat;
			}
		}
	}
	catch (sql::SQLException &ex)
	{
		m_lasterror = ex.what();
	}
	return m_pConnection;
}

void CSqlQuery_Mysql::setAutoCommit(bool autoCommit)
{
	if (m_pConnection == nullptr) return;
	try
	{
		m_pConnection->setAutoCommit(autoCommit);
	}
	catch (sql::SQLException &ex)
	{
		m_lasterror = ex.what();
	}
}

bool CSqlQuery_Mysql::prepare(const std::string &sql)
{
	m_sql = sql;
	return true;
}

bool CSqlQuery_Mysql::executeUpdate(const std::string &sql)
{
	if (m_pStat == nullptr) return false;
	try
	{
		if (!sql.empty())
		{
			m_pStat->executeUpdate(sql.c_str());
		}
		else
		{
			m_pStat->executeUpdate(m_sql.c_str());
		}
	}
	catch (sql::SQLException &ex)
	{
		LERROR("executeUpdate error: {}", ex.what());
		m_lasterror = ex.what();
		return false;
	}
	return true;
}

bool CSqlQuery_Mysql::executeQuery(const std::string &sql)
{
	if (m_pStat == nullptr) return false;
	try
	{
		if (m_pRset)
		{
			m_nIndex = 0;
			m_pRset->close();
			delete m_pRset;
		}
		if (!sql.empty())
		{
			m_pRset = m_pStat->executeQuery(sql.c_str());
		}
		else
		{
			m_pRset = m_pStat->executeQuery(m_sql.c_str());
		}
	}
	catch (sql::SQLException &ex)
	{
		LERROR("executeQuery error: {}", ex.what());
		m_lasterror = ex.what();
		return false;
	}
	return true;
}

bool CSqlQuery_Mysql::execute(const std::string &sql)
{
	if (m_pStat == nullptr) return false;
	try
	{
		if (!sql.empty())
		{
			m_pStat->execute(sql.c_str());
		}
		else
		{
			m_pStat->execute(m_sql.c_str());
		}
	}
	catch (sql::SQLException &ex)
	{
		LERROR("execute error: {}", ex.what());
		m_lasterror = ex.what();
		return false;
	}
	return true;
}

bool CSqlQuery_Mysql::exec()
{
	return true;
}

bool CSqlQuery_Mysql::next()
{
	if (m_pRset == nullptr) return false;
	try
	{
		if (m_pRset->next())
		{
			return true;
		}
	}
	catch (sql::SQLException &ex)
	{
		LERROR("{}", ex.what());
		m_lasterror = ex.what();
	}
	return false;
}



CVariant CSqlQuery_Mysql::value(const int &index, const CVariant &defvalue)
{
	CVariant vat = defvalue;
	try
	{
		switch (defvalue.m_v.which())
		{
		case 0: vat.setValue<int>(m_pRset->getInt(index)); break;
		case 1: vat.setValue<unsigned int>(m_pRset->getUInt(index)); break;
		case 2: vat.setValue<float>(m_pRset->getDouble(index)); break;
		case 3: vat.setValue<double>(m_pRset->getDouble(index)); break;
		case 4: vat.setValue<char>(m_pRset->getUInt(index)); break;
		case 5: vat.setValue<std::string>(m_pRset->getString(index).c_str()); break;
		case 6: vat.setValue<std::string>(m_pRset->getString(index).c_str()); break;
		default:break;
		}
	}
	catch (sql::SQLException &ex)
	{
		LERROR("getvalue() error {}", ex.what());
		m_lasterror = ex.what();
	}
	return vat;
}

CVariant CSqlQuery_Mysql::value(const std::string& name, const CVariant &defvalue)
{
	CVariant vat = defvalue;
	try
	{
		switch (defvalue.m_v.which())
		{
		case 0: vat.setValue<int>(m_pRset->getInt(name)); break;
		case 1:vat.setValue<unsigned int>(m_pRset->getUInt(name)); break;
		case 2:vat.setValue<float>(m_pRset->getDouble(name)); break;
		case 3:vat.setValue<double>(m_pRset->getDouble(name)); break;
		case 4:vat.setValue<char>(m_pRset->getUInt(name)); break;
		case 5:vat.setValue<std::string>(m_pRset->getString(name)); break;
		case 6:vat.setValue<std::string>(m_pRset->getString(name)); break;
		default:break;
		}
	}
	catch (sql::SQLException &ex)
	{
		LERROR("getvalue() error {}", ex.what());
		m_lasterror = ex.what();
	}
	return vat;
}
void CSqlQuery_Mysql::bindValue(const int &nIndex, const int &x)
{
	if (m_sql.empty())
	{
		return;
	}
	char s[16] = { 0 };
	sprintf(s, ":%d", nIndex);
	size_t index = m_sql.find(s, 0);
	if (index != std::string::npos)
	{
		char v[16] = { 0 };
		sprintf(v, "%d", x);
		m_sql.replace(index, strlen(s), v, 0, strlen(v));
	}
}

void CSqlQuery_Mysql::bindValue(const int &nIndex, const double &x)
{
	if (m_sql.empty())
	{
		return;
	}
	char s[16] = { 0 };
	sprintf(s, ":%d", nIndex);
	size_t index = m_sql.find(s, 0);
	if (index != std::string::npos)
	{
		char v[16] = { 0 };
		sprintf(v, "%f", x);
		m_sql.replace(index, strlen(s), v, 0, strlen(v));
	}
}

void CSqlQuery_Mysql::bindValue(const int &nIndex, const float &x)
{
	if (m_sql.empty())
	{
		return;
	}
	char s[16] = { 0 };
	sprintf(s, ":%d", nIndex);
	size_t index = m_sql.find(s, 0);
	if (index != std::string::npos)
	{
		char v[16] = { 0 };
		sprintf(v, "%f", x);
		m_sql.replace(index, strlen(s), v, 0, strlen(v));
	}
}

void CSqlQuery_Mysql::bindValue(const int &nIndex, const unsigned long &x)
{
	if (m_sql.empty())
	{
		return;
	}
	char s[16] = { 0 };
	sprintf(s, ":%d", nIndex);
	size_t index = m_sql.find(s, 0);
	if (index != std::string::npos)
	{
		char v[16] = { 0 };
		sprintf(v, "%lu", x);
		m_sql.replace(index, strlen(s), v, 0, strlen(v));
	}
}

void CSqlQuery_Mysql::bindValue(const int &nIndex, const std::string &x)
{
	if (m_sql.empty())
	{
		return;
	}
	char s[16] = { 0 };
	sprintf(s, ":%d", nIndex);
	size_t index = m_sql.find(s, 0);
	if (index != std::string::npos)
	{
		m_sql.replace(index, strlen(s), x);
	}
}

