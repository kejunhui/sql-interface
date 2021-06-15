#pragma once
#include "boost/variant.hpp"

class CVariant
{
public:
	CVariant() {}
	CVariant(int n) { m_v = n; }
	CVariant(unsigned int n) { m_v = n; }
	CVariant(float f) { m_v = f; }
	CVariant(double d) { m_v = d; }
	CVariant(char c) { m_v = c; }
	CVariant(const char *p) { m_v = p; }
	CVariant(std::string str) { m_v = str; }
	CVariant(const CVariant &other) { m_v = other.m_v; }
	~CVariant() {}

	inline CVariant& operator=(const CVariant &other)
	{
		m_v = other.m_v;
	}
	inline CVariant(CVariant &&other)
	{
		m_v = other.m_v;
	}
	inline CVariant &operator=(CVariant &&other)
	{
		m_v = other.m_v;
	}

	template<typename T>
	inline void setValue(const T &value)
	{
		m_v = value;
	}

	template<typename T>
	inline T value() const
	{
		return boost::get<T>(m_v);
	}

	inline int toInt() const
	{
		return boost::get<int>(m_v);
	}
	inline unsigned int toUInt()
	{
		return boost::get<unsigned int>(m_v);
	}
	inline float toFloat() const
	{
		return boost::get<float>(m_v);
	}
	inline double toDouble() const
	{
		return boost::get<double>(m_v);
	}
	inline char toChar() const
	{
		return boost::get<char>(m_v);
	}
	inline std::string toString() const
	{
		return boost::get<std::string>(m_v);
	}
	
public:
	boost::variant<int, unsigned int, float, double, char, const char *, std::string> m_v;
};