#pragma once
#include <string>
#include <atomic>
#include "sqldriver.h"

class CSqlDatabase
{
public:
	enum DATABASETYPE
	{
		DB_MYSQL,
		DB_ORACLE,
		DB_SQLSERVER
	};

	explicit CSqlDatabase(DATABASETYPE type);
	~CSqlDatabase();

	void setSource(const std::string &source);
	void setDatabaseName(const std::string &database);
	void setUserName(const std::string &user);
	void setPassword(const std::string &password);

	bool open(int size = 1);
	bool close();
	bool isOpen();

	bool commit();
	bool rollback();

	CSqlQuery * getQuery();
	void releaseQuery(CSqlQuery * pQuery, bool bRelease = true);
private:
	std::atomic<bool>				m_bOpen;
	DATABASETYPE					m_etype;
	CSqlDatabasePrivate				m_private;
	std::unique_ptr<CSqlConnPool>	m_pConnPool;
};


