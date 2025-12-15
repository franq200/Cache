#pragma once
#include <unordered_map>
#include <exception>
#include <chrono>
#include <thread>
#include <algorithm>
#include <mutex>

template < typename Key, typename Value>
class Cache
{
public:
	Cache(size_t maxSize);
	Cache() = default;
	~Cache();
	void Put(const Key& key, const Value& value, size_t ttlInMs = 15000);
	const Value& Get(const Key& key);
	bool Contains(const Key& key) const;
private:
	struct CacheItem
	{
		Value value;
		std::chrono::time_point<std::chrono::steady_clock> timestamp;
		std::chrono::time_point<std::chrono::steady_clock> expiryTime;
		void UpdateTimestamp()
		{
			timestamp = std::chrono::steady_clock::now();
		}
	};
	void RemoveOldestValue();
	void CleanupExpiredItems();
	std::unordered_map<Key, CacheItem> data_;
	const size_t maxSize_ = 64;
	std::thread thread_;
	std::mutex mutex_;
};

template<typename Key, typename Value>
inline Cache<Key, Value>::~Cache()
{
	if (thread_.joinable())
	{
		thread_.join();
	}
}

template<typename Key, typename Value>
inline void Cache<Key, Value>::Put(const Key& key, const Value& value, size_t ttlInMs)
{
	std::unique_lock<std::mutex> lock(mutex_);
	if (data_.find(key) == data_.end())
	{
		if (data_.size() >= maxSize_)
		{
			RemoveOldestValue();
		}
		auto now = std::chrono::steady_clock::now();
		data_[key] = CacheItem{ value , now , now + std::chrono::milliseconds(ttlInMs)};
	}
	else
	{
		throw std::exception("Key already exists in cache");
	}
}

template<typename Key, typename Value>
inline Cache<Key, Value>::Cache(size_t maxSize) :
	maxSize_(maxSize)
{
	thread_ = std::thread(&Cache<Key, Value>::CleanupExpiredItems, this);
}

template<typename Key, typename Value>
inline const Value& Cache<Key, Value>::Get(const Key& key)
{
	std::unique_lock<std::mutex> lock(mutex_);
	auto it = data_.find(key);
	if (it == data_.end())
	{
		throw std::exception("Key not found in cache");
	}
	it->second.UpdateTimestamp();
	return it->second.value;
}

template<typename Key, typename Value>
inline bool Cache<Key, Value>::Contains(const Key& key) const
{
	std::unique_lock<std::mutex> lock(mutex_);
	return data_.find(key) != data_.end();
}

template<typename Key, typename Value>
inline void Cache<Key, Value>::RemoveOldestValue()
{
	if (data_.empty())
	{
		return;
	}
	auto oldest = data_.begin();
	for (auto it = data_.begin(); it != data_.end(); it++)
	{
		if (it->second.timestamp < oldest->second.timestamp)
		{
			oldest = it;
		}
	}
	data_.erase(oldest);
}

template<typename Key, typename Value>
inline void Cache<Key, Value>::CleanupExpiredItems()
{
	while (true)
	{
		auto now = std::chrono::steady_clock::now();
		std::unique_lock<std::mutex> lock(mutex_);
		for (auto it = data_.begin(); it != data_.end();)
		{
			if (now >= it->second.expiryTime)
			{
				it = data_.erase(it);
			}
			else
			{
				++it;
			}
		}
	}
}
