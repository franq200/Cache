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
	mutex_.lock();
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
	mutex_.lock();
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
	mutex_.lock();
	return data_.find(key) != data_.end();
}

template<typename Key, typename Value>
inline void Cache<Key, Value>::RemoveOldestValue()
{
	mutex_.lock();
	auto oldest = data_.begin();
	data_.erase(std::find_if(data_.begin(), data_.end(), [oldest](const auto& el) {return el.second.timestamp < oldest->second.timestamp; }));
}

template<typename Key, typename Value>
inline void Cache<Key, Value>::CleanupExpiredItems()
{
	while (true)
	{
		auto now = std::chrono::steady_clock::now();
		mutex_.lock();
		data_.erase(std::remove_if(data_.begin(), data_.end(), [now](const auto& el) {return now >= el.second.expiryTime; }), data_.end());
	}
}
