#include <iostream>
#include "sqldriver_oracle.h"
#include "log.h"

using namespace std;
CSqlConnPool_Oracle::CSqlConnPool_Oracle(const CSqlDatabasePrivate &pri):CSqlConnPool(pri), m_env(nullptr), m_connPool(nullptr)
{

}

CSqlConnPool_Oracle::~CSqlConnPool_Oracle()
{
	DestoryConnPool();
}

void CSqlConnPool_Oracle::DestoryConnPool()
{
	try
	{
		m_env->terminateStatelessConnectionPool(m_connPool);
		Environment::terminateEnvironment(m_env);
	}
	catch (oracle::occi::SQLException &ex)
	{
		LERROR("release connection error: {}", ex.what());
		m_lasterror = ex.what();
	}
}

bool CSqlConnPool_Oracle::InitConnpool(int maxSize)
{
	try
	{
		LDEBUG("InitConnpool {}", maxSize);
		m_env = Environment::createEnvironment(Environment::THREADED_MUTEXED);
		m_connPool = m_env->createStatelessConnectionPool(m_private.user, m_private.password, m_private.source, maxSize, 0, 2);
		m_connPool->setTimeOut(900);

		StatelessConnectionPool::BusyOption BusyOption = StatelessConnectionPool::FORCEGET;
		m_connPool->setBusyOption(BusyOption);
	}
	catch (oracle::occi::SQLException &ex)
	{
		LERROR("create connPool error: {}", ex.what());
		m_lasterror = ex.what();
		return false;
	}
	return true;
}

void * CSqlConnPool_Oracle::GetConnection()
{
	oracle::occi::Connection *conn = nullptr;
	try
	{
		conn = m_connPool->getConnection();
	}
	catch (oracle::occi::SQLException &ex)
	{
		LERROR("get conn error: {}", ex.what());
		m_lasterror = ex.what();
	}
	return conn;
}

void CSqlConnPool_Oracle::ReleaseConnection(void *conn, bool bRelease)
{
	try
	{
		//conn->flushCache();
		m_connPool->releaseConnection((oracle::occi::Connection*)conn);
	}
	catch (oracle::occi::SQLException &ex)
	{
		LERROR("release connection error: {}", ex.what());
		m_lasterror = ex.what();
	}
}


//************************************ CSqlQuery_Oracle *************************************//
CSqlQuery_Oracle::CSqlQuery_Oracle(void * conn):m_pConnection(nullptr), m_pStat(nullptr), m_pRset(nullptr)
{
	try 
	{
		m_nIndex = 0;
		m_pConnection = static_cast<oracle::occi::Connection*>(conn);
		m_pStat = m_pConnection->createStatement();
		
	}
	catch (oracle::occi::SQLException &ex)
	{
		LERROR("init oracle query failed: {}", ex.what());
		m_lasterror = ex.what();
	}	
}

CSqlQuery_Oracle::~CSqlQuery_Oracle() 
{
	
}

void * CSqlQuery_Oracle::release()
{
	try
	{
		if (m_pConnection)
		{
			if (m_pRset)
			{
				m_pStat->closeResultSet(m_pRset);
			}
			if (m_pStat)
			{
				m_pConnection->terminateStatement(m_pStat);
			}
		}
	}
	catch (oracle::occi::SQLException &ex)
	{
		m_lasterror = ex.what();
	}
	return m_pConnection;
}

void CSqlQuery_Oracle::setAutoCommit(bool autoCommit)
{
	if (m_pStat == nullptr) return;
	try
	{
		m_pStat->setAutoCommit(autoCommit);
	}
	catch (oracle::occi::SQLException &ex)
	{
		m_lasterror = ex.what();
	}
}

bool CSqlQuery_Oracle::prepare(const std::string &sql)
{
	if (m_pStat == nullptr) return false;
	try
	{
		m_pStat->setSQL(sql);
	}
	catch (oracle::occi::SQLException &ex)
	{
		m_lasterror = ex.what();
		return false;
	}
	return true;
}

bool CSqlQuery_Oracle::executeUpdate(const std::string &sql)
{
	if (m_pStat == nullptr) return false;
	try
	{
		m_pStat->executeUpdate(sql);
	}
	catch (oracle::occi::SQLException &ex)
	{
		LERROR("executeUpdate error: {}", ex.what());
		m_lasterror = ex.what();
		return false;
	}
	return true;
}

bool CSqlQuery_Oracle::executeQuery(const std::string &sql)
{
	if (m_pStat == nullptr) return false;
	try
	{
		if (m_pRset)
		{
			m_nIndex = 0;
			m_pStat->closeResultSet(m_pRset);
			m_pRset = nullptr;
		}
		m_pRset = m_pStat->executeQuery(sql);
	}
	catch (oracle::occi::SQLException &ex)
	{
		LERROR("executeQuery error: {}", ex.what());
		m_lasterror = ex.what();
		return false;
	}
	return true;
}

bool CSqlQuery_Oracle::execute(const std::string &sql)
{
	if (m_pStat == nullptr) return false;
	try
	{
		oracle::occi::Statement::Status s = m_pStat->execute(sql);
	}
	catch (oracle::occi::SQLException &ex)
	{
		LERROR("execute error: {}", ex.what());
		m_lasterror = ex.what();
		return false;
	}
	return true;
}

bool CSqlQuery_Oracle::exec()
{
	if (m_pStat == nullptr) return false;
	try
	{
		oracle::occi::Statement::Status s = m_pStat->execute();
	} 
	catch (oracle::occi::SQLException &ex)
	{
		LERROR("exec error: {}", ex.what());
		m_lasterror = ex.what();
		return false;
	}
	return true;
}

bool CSqlQuery_Oracle::next()
{
	if (m_pRset == nullptr) return false;
	try
	{
		if (m_pRset->next())
		{
			return true;
		}
	}
	catch (oracle::occi::SQLException &ex)
	{
		LERROR("{}", ex.what());
		m_lasterror = ex.what();
	}
	return false;
}

CVariant CSqlQuery_Oracle::value(const int &index, const CVariant &defvalue)
{
	CVariant vat = defvalue;
	try
	{
		switch (defvalue.m_v.which())
		{
		case 0: vat.setValue<int>(m_pRset->getInt(index)); break;
		case 1:vat.setValue<unsigned int>(m_pRset->getUInt(index)); break;
		case 2:vat.setValue<float>(m_pRset->getFloat(index)); break;
		case 3:vat.setValue<double>(m_pRset->getDouble(index)); break;
		case 5:vat.setValue<std::string>(m_pRset->getString(index)); break;
		case 6:vat.setValue<std::string>(m_pRset->getString(index)); break;
		//case CDATATYPE::_DATE:vat.setValue<std::string>(m_pRset->getDate(index).toText("yyyymmddhh24miss")); break;
		default:break;
		}
	}
	catch (oracle::occi::SQLException &ex)
	{
		LERROR("getvalue() error {}", ex.what());
		m_lasterror = ex.what();
	}
	return vat;
}

CVariant CSqlQuery_Oracle::value(const std::string& name, const CVariant &eType)
{
	// oracle 没有按名称取值的接口
	CVariant vat;
	return vat;
}

void CSqlQuery_Oracle::bindValue(const int &nIndex, const int &x)
{
	if (m_pStat == nullptr) return;
	try
	{
		m_pStat->setInt(nIndex, x);
	}
	catch (oracle::occi::SQLException &ex)
	{
		m_lasterror = ex.what();
	}
}

void CSqlQuery_Oracle::bindValue(const int &nIndex, const double &x)
{
	if (m_pStat == nullptr) return;
	try
	{
		m_pStat->setDouble(nIndex, x);
	}
	catch (oracle::occi::SQLException &ex)
	{
		m_lasterror = ex.what();
	}
}

void CSqlQuery_Oracle::bindValue(const int &nIndex, const float &x)
{
	if (m_pStat == nullptr) return;
	try
	{
		m_pStat->setFloat(nIndex, x);
	}
	catch (oracle::occi::SQLException &ex)
	{
		m_lasterror = ex.what();
	}
}

void CSqlQuery_Oracle::bindValue(const int &nIndex, const unsigned long &x)
{
	if (m_pStat == nullptr) return;
	try
	{
		m_pStat->setInt(nIndex, x);
	}
	catch (oracle::occi::SQLException &ex)
	{
		m_lasterror = ex.what();
	}
}

void CSqlQuery_Oracle::bindValue(const int &nIndex, const std::string &x)
{
	if (m_pStat == nullptr) return;
	try
	{
		m_pStat->setString(nIndex, x);
	}
	catch (oracle::occi::SQLException &ex)
	{
		m_lasterror = ex.what();
	}
}