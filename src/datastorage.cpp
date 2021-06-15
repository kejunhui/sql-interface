#include "sqldatabase.h"
#include "datastorage.h"
#include "configuration.h"
#include "log.h"

DataStorage* DataStorage::m_pInstance = nullptr;
DataStorage* DataStorage::GetInstance()
{
	if (m_pInstance == nullptr)
	{
		m_pInstance = new DataStorage();
	}
	return m_pInstance;
}

void DataStorage::ReleaseInstance()
{
	if (m_pInstance) delete m_pInstance;
	m_pInstance = nullptr;
}


DataStorage::DataStorage()
{
	Init();
}

DataStorage::~DataStorage()
{
	End();
}

int DataStorage::Init()
{
	m_pDatabase = std::make_unique<CSqlDatabase>(CSqlDatabase::DB_MYSQL);
	return 0;
}

int DataStorage::End()
{
	m_pDatabase->close();
	return 0;
}

int DataStorage::SetConnSQL(const std::string &user, const std::string &password, const std::string &database_name, const std::string &database_source)
{
	m_pDatabase->setDatabaseName(database_name);
	m_pDatabase->setSource(database_source);
	m_pDatabase->setUserName(user);
	m_pDatabase->setPassword(password);
	m_pDatabase->open();
	return 0;
}

int DataStorage::ResetAllConnection()
{
	m_pDatabase->close();
	m_pDatabase->open();
	return 0;
}

int DataStorage::ReadStation(VECSTATION& data, const int &protocol)
{
	CSqlQuery *query = m_pDatabase->getQuery();
	if (!query)
	{
		return -1;
	}
	//query->prepare("SELECT id,name,dataformat,hostname,port,protocol,mount_point,username,password,antenna_name,antenna_height FROM station WHERE subgroup=:1");
	//query->bindValue(1, stationgroup);
	query->prepare("SELECT s.id, s.hostname, s.`port`, s.username, s.`password`, s.protocol, t.`name`, t.dataformat, t.antenna_name, t.antenna_height, m.mountpoint\
		FROM `stream` s, `station` t, `mountpoint` m where s.station_id = t.id and s.mount_id = m.id and s.protocol =:1");
	query->bindValue(1, protocol);
	if (!query->executeQuery())
	{
		std::string error = query->getLasterror();
		m_pDatabase->releaseQuery(query, false);
		return -1;
	}
	while (query->next())
	{
		STATIONINFO station;
		station.nId = query->value(1, 0).toInt();
		station.strHostName = query->value(2, "").toString();
		station.nPort = query->value(3, 0).toUInt();
		station.strUserName = query->value(4, "").toString();
		station.strPassword = query->value(5, "").toString();
		station.nProtocol = query->value(6, 0).toUInt();
		station.strStationName = query->value(7, "").toString();
		station.nDeviceType = STATIONINFO::getDeviceType(query->value(8, "").toString());
		station.strAntennaName = query->value(9,"").toString();
		station.AntennaHeight = query->value(10, 0).toUInt();
		station.strMountPoint = query->value(11, "").toString();
		data.push_back(station);
	}
	m_pDatabase->releaseQuery(query);
	return 0;
}

