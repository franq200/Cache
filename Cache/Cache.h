#pragma once
#include <unordered_map>
#include <stdexcept>
#include <chrono>
#include <thread>
#include <algorithm>
#include <mutex>
#include <atomic>

class ITimeProvider
{
public:
	bool virtual Tick() = 0;
	virtual ~ITimeProvider() = default;
	virtual std::chrono::time_point<std::chrono::steady_clock> Now() = 0;
};

class TimeProvider : public ITimeProvider
{
public:
	bool Tick() override
	{
		counter_++;
		if (counter_ == treshold_)
		{
			counter_ = 0;
			return true;
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::seconds(1));
			return false;
		}
	}
	std::chrono::time_point<std::chrono::steady_clock> Now() override
	{
		return std::chrono::steady_clock::now();
	}
private:
	int counter_ = 0;
	const int treshold_ = 120;
};

template < typename Key, typename Value>
class Cache
{
public:
	Cache(size_t maxSize, ITimeProvider& timerProvider);
	Cache(ITimeProvider& timerProvider);
	~Cache();
	void Put(const Key& key, const Value& value, size_t ttlInMs = 15000);
	Value Get(const Key& key);
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
	ITimeProvider& timeProvider_;
	std::unordered_map<Key, CacheItem> data_;
	const size_t maxSize_ = 64;
	std::thread cleanupThread_;
	mutable std::mutex mutex_;
	std::atomic<bool> running_{ true };
};

template<typename Key, typename Value>
inline Cache<Key, Value>::~Cache()
{
	running_.store(false);
	if (cleanupThread_.joinable())
	{
		cleanupThread_.join();
	}
}

template<typename Key, typename Value>
inline void Cache<Key, Value>::Put(const Key& key, const Value& value, size_t ttlInMs)
{
	std::unique_lock<std::mutex> lock(mutex_);
	auto now = timeProvider_.Now();
	data_.try_emplace(key, CacheItem{ value, now, now + std::chrono::milliseconds(ttlInMs) });
	if (data_.size() > maxSize_)
	{
		RemoveOldestValue();
	}
}

template<typename Key, typename Value>
inline Cache<Key, Value>::Cache(size_t maxSize, ITimeProvider& timerProvider) :
	maxSize_(maxSize), timeProvider_(timerProvider)
{
	cleanupThread_ = std::thread(&Cache<Key, Value>::CleanupExpiredItems, this);
}

template<typename Key, typename Value>
inline Cache<Key, Value>::Cache(ITimeProvider& timerProvider):
	timeProvider_(timerProvider)
{
	cleanupThread_ = std::thread(&Cache<Key, Value>::CleanupExpiredItems, this);
}

template<typename Key, typename Value>
inline Value Cache<Key, Value>::Get(const Key& key)
{
	std::unique_lock<std::mutex> lock(mutex_);
	auto it = data_.find(key);
	if (it == data_.end())
	{
		throw std::out_of_range("Key not found in cache");
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
	while (running_)
	{
		if (timeProvider_.Tick())
		{
			auto now = timeProvider_.Now();

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
}
