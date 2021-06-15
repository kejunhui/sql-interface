#pragma once
#include <memory>
#include <string>
#include "definition.h"

class CSqlDatabase;
class ThreadPool;
class DataStorage
{
public:
	static DataStorage* GetInstance();
	static void ReleaseInstance();

protected:
	DataStorage();
	~DataStorage();
public:
	int Init();
	int End();

	int SetConnSQL(const std::string &user, const std::string &password, const std::string &database_name, const std::string &database_source);
	int ResetAllConnection();
	int ReadStation(VECSTATION& pData, const int &protocol);

private:
	static DataStorage			* m_pInstance;
	std::unique_ptr<CSqlDatabase> m_pDatabase;
};
