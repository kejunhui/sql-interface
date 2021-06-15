#pragma once
#include <list>
#include <mutex>
#include <memory>
#include <vector>
#include <unordered_map>
#include <utility>
#include <atomic>
#include "boost/thread/mutex.hpp"
#include "boost/thread/shared_mutex.hpp"

using namespace std;
template<typename Key, typename Value, int n = 199, typename Hash = std::hash<Key>>
class threadsafe_lookup_table // ֻ���ÿ��bucket��������ȫ�ֵ�vector���������Ӷ��������������ȡ���ʹ��map���Ǽ򵥵Ĳ�������Ҫ������map����,Ƶ����д���ø�����
{
private:
	class bucket_type
	{
	private:
		typedef std::unordered_map<Key, Value> bucket_data;
		typedef typename bucket_data::iterator bucket_iterator;
		typedef typename bucket_data::const_iterator bucket_const_iterator;

		bucket_data data;
		mutable boost::shared_mutex _mutex;

		bucket_iterator find_entry_for(Key const& key)
		{
			return data.find(key);
		}

		bucket_const_iterator find_entry_for(Key const& key) const //����Ҫ���ط�const�ĵ��������Խ����ݷ��õ���const��������
		{
			return data.find(key);
		}
	public:

		Value value_for(Key const& key, Value const& default_value) const  //�����������ȡʱ��������
		{
			boost::shared_lock<boost::shared_mutex> lock(_mutex);
			bucket_const_iterator found_entry = find_entry_for(key);
			return (found_entry == data.end()) ? default_value : found_entry->second;
		}

		void add_or_update_mapping(Key const& key, Value const& value) //���޸�ʱҲ��������
		{
			boost::unique_lock<boost::shared_mutex> lock(_mutex);
			bucket_iterator found_entry = find_entry_for(key);
			if (found_entry == data.end())
			{
				data[key] = value;
			}
			else
			{
				found_entry->second = value; //���key�Ѿ����ھ͸���value
			}
		}

		void remove_mapping(Key const& key)
		{
			boost::unique_lock<boost::shared_mutex> lock(_mutex);
			bucket_iterator const found_entry = find_entry_for(key);
			if (found_entry != data.end())
			{
				data.erase(found_entry);
			}
		}

		template<typename Predicate>
		void remove_if(Predicate &&p)
		{
			boost::unique_lock<boost::shared_mutex> lock(_mutex);
			for (bucket_iterator it = data.begin(); it != data.end();)
			{
				if (p(it->second)) it = data.erase(it);
				else ++it;
			}
		}

		template<typename Function>
		void for_each(Function &&f)
		{
			boost::unique_lock<boost::shared_mutex> lock(_mutex);
			for (bucket_iterator it = data.begin(); it != data.end(); it++)
			{
				f(it->second);
			}
		}
	};

	std::vector<std::unique_ptr<bucket_type> > buckets; //����Ͱ�ı���
	Hash hasher;

	bucket_type& get_bucket(Key const& key) const  //��ȡ���������ı����Բ���Ҫ��
	{
		std::size_t const bucket_index = hasher(key) % buckets.size();
		return *buckets[bucket_index];
	}

public:
	threadsafe_lookup_table(
		unsigned num_buckets = n, Hash const& hasher_ = Hash()) :
		buckets(num_buckets), hasher(hasher_) //��ʼ���������������ϣЧ���������ѡ��29��ʼ��
	{
		for (unsigned i = 0; i < num_buckets; ++i)
		{
			buckets[i].reset(new bucket_type);
		}
	}

	threadsafe_lookup_table(threadsafe_lookup_table const& other) = delete;
	threadsafe_lookup_table const& operator=(threadsafe_lookup_table const& other) = delete;//Ϊ�˼򻯴����ֹ�����͸�ֵ

	Value value_for(Key const& key, Value const& default_value = Value())
	{
		return get_bucket(key).value_for(key, default_value);
	}

	void add_or_update_mapping(Key const& key, Value const& value)//�쳣��ȫ�ģ���Ϊadd_or_update_mapping���쳣��ȫ��
	{
		get_bucket(key).add_or_update_mapping(key, value);
	}

	void remove_mapping(Key const& key)
	{
		get_bucket(key).remove_mapping(key);
	}

	template<typename Predicate>
	void remove_if(Predicate &&p)
	{
		for (unsigned i = 0; i < buckets.size(); ++i)
		{
			(*buckets[i]).remove_if(std::forward<Predicate>(p));
		}
	}

	template<typename Function>
	void for_each(Function &&f)//���map��ÿ��Ԫ��ִ��f
	{
		for (unsigned i = 0; i < buckets.size(); ++i)
		{
			(*buckets[i]).for_each(std::forward<Function>(f));
		}
	}
};


template <typename T>
class FreeList
{
public:
	FreeList() {}
	~FreeList()
	{
		m_mutex.lock();
		for (typename std::list<T*>::iterator it = m_list.begin(); it != m_list.end();)
		{
			if (*it) delete *it;
			it = m_list.erase(it);
		}
		m_mutex.unlock();
	}
	void initSize(int n)
	{
		m_mutex.lock();
		for (int i = 0; i<n; i++)
		{
			T *  t = new T();
			m_list.push_back(t);
		}
		m_mutex.unlock();
	}
	T *getFreeElem()
	{
		T *t = NULL;
		m_mutex.lock();
		if (!m_list.empty())
		{
			t = m_list.front();
			m_list.pop_front();
		}
		else t = new T();
		m_mutex.unlock();
		return t;
	}
	void putFreeElem(T * t)
	{
		m_mutex.lock();
		m_list.push_front(t);
		m_mutex.unlock();
	}
private:

	std::list<T *> m_list;
	std::mutex m_mutex;
};


