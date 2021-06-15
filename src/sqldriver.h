#pragma once
#include <string>
#include "variant.h"

// ˽������
struct CSqlDatabasePrivate
{
	std::string source;			// ����Դ
	std::string database;		// ���ݿ�����
	std::string user;			// �û���
	std::string password;		// ����
};

// ���ݿ����ӳ�
class CSqlConnPool
{
public:
	explicit CSqlConnPool(const CSqlDatabasePrivate &pri) :m_private(pri) {}
	~CSqlConnPool() {}

	virtual void * GetConnection() = 0;										// ��ȡ���ݿ�����
	virtual void DestoryConnPool() = 0;										// �������ݿ����ӳ�	
	virtual void ReleaseConnection(void* conn, bool bRelease = true) = 0;	// �ͷ����ӻ����ӳ�
	virtual bool InitConnpool(int maxSize) = 0;								// ��ʼ�����ӳ�

protected:
	std::string				 m_lasterror;
	CSqlDatabasePrivate		 m_private;
};


// ��������࣬�̲߳���ȫ
class CSqlQuery
{
public:
	explicit CSqlQuery() :m_nIndex(0) {}
	virtual ~CSqlQuery() {}
	virtual void * release() = 0;
	virtual void setAutoCommit(bool autoCommit = true) = 0;				// �Զ��ύ
	virtual bool prepare(const std::string &sql = "") = 0;				// sql ���׼��
	virtual bool executeUpdate(const std::string &sql = "") = 0;		// ִ��update
	virtual bool executeQuery(const std::string &sql = "") = 0;			// ִ�в�ѯ
	virtual bool execute(const std::string &sql = "") = 0;				// ִ��sql
	virtual bool exec() = 0;
	virtual bool next() = 0;

	virtual CVariant value(const int &index, const CVariant &defvalue) = 0;			// ���±��ȡ�ֶ�ֵ
	virtual CVariant value(const std::string& name, const CVariant &defvalue) = 0;	// ���ֶ�����ȡ�ֶ�ֵ

																					// ��sql ����
	std::string getLasterror() { return m_lasterror; }
	template<typename first_t, typename...rest_t>
	void bindValues(first_t first, rest_t... rest)
	{
		bindValue(++m_nIndex, first);
		bindValues(rest...);
	}
	virtual void bindValue() {}
	virtual void bindValue(const int &nIndex, const int &x) = 0;
	virtual void bindValue(const int &nIndex, const double &x) = 0;
	virtual void bindValue(const int &nIndex, const float &x) = 0;
	virtual void bindValue(const int &nIndex, const unsigned long &x) = 0;
	virtual void bindValue(const int &nIndex, const std::string &x) = 0;
protected:
	int			   m_nIndex;		// �󶨲������±�
	std::string    m_lasterror;
};