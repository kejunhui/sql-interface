#pragma once
#include <string>
#include "variant.h"

// 私有配置
struct CSqlDatabasePrivate
{
	std::string source;			// 连接源
	std::string database;		// 数据库名称
	std::string user;			// 用户名
	std::string password;		// 密码
};

// 数据库连接池
class CSqlConnPool
{
public:
	explicit CSqlConnPool(const CSqlDatabasePrivate &pri) :m_private(pri) {}
	~CSqlConnPool() {}

	virtual void * GetConnection() = 0;										// 或取数据库连接
	virtual void DestoryConnPool() = 0;										// 销毁数据库连接池	
	virtual void ReleaseConnection(void* conn, bool bRelease = true) = 0;	// 释放连接回连接池
	virtual bool InitConnpool(int maxSize) = 0;								// 初始化连接池

protected:
	std::string				 m_lasterror;
	CSqlDatabasePrivate		 m_private;
};


// 事务操作类，线程不安全
class CSqlQuery
{
public:
	explicit CSqlQuery() :m_nIndex(0) {}
	virtual ~CSqlQuery() {}
	virtual void * release() = 0;
	virtual void setAutoCommit(bool autoCommit = true) = 0;				// 自动提交
	virtual bool prepare(const std::string &sql = "") = 0;				// sql 语句准备
	virtual bool executeUpdate(const std::string &sql = "") = 0;		// 执行update
	virtual bool executeQuery(const std::string &sql = "") = 0;			// 执行查询
	virtual bool execute(const std::string &sql = "") = 0;				// 执行sql
	virtual bool exec() = 0;
	virtual bool next() = 0;

	virtual CVariant value(const int &index, const CVariant &defvalue) = 0;			// 按下标获取字段值
	virtual CVariant value(const std::string& name, const CVariant &defvalue) = 0;	// 按字段名获取字段值

																					// 绑定sql 参数
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
	int			   m_nIndex;		// 绑定参数的下标
	std::string    m_lasterror;
};