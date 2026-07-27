#include "Cache.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>

int main(int argc, char** argv) {
	::testing::InitGoogleTest(&argc, argv); // inicjalizacja Google Test
	return RUN_ALL_TESTS();                 // uruchomienie wszystkich testów
}

class TimeProviderMock : public ITimeProvider
{
public:
	MOCK_METHOD(bool, Tick, (), (override));
	MOCK_METHOD(std::chrono::time_point<std::chrono::steady_clock>, Now, (), (override));
};

class CacheTest : public ::testing::Test {
protected:
	::testing::StrictMock<TimeProviderMock> timeProvider_;
};

TEST_F(CacheTest, PutOneItemContainsTrueGetTrue)
{
	auto now = std::chrono::steady_clock::now();

	EXPECT_CALL(timeProvider_, Now())
		.Times(::testing::AtLeast(1))
		.WillRepeatedly(::testing::Return(now));

	Cache<std::string, std::string> cache(2, timeProvider_);
	

	cache.Put("key1", "value1");

	EXPECT_TRUE(cache.Contains("key1"));

	auto value = cache.Get("key1");
	EXPECT_EQ(value, "value1");
}

TEST_F(CacheTest, PutOneItemContainsFalseGetTrue)
{
	Cache<std::string, std::string> cache(2, timeProvider_);
	cache.Put("key1", "value1");
	EXPECT_FALSE(cache.Contains("key"));

	auto value = cache.Get("key1");
	EXPECT_EQ(value, "value1");
}

TEST_F(CacheTest, PutOneItemContainsTrueGetFalse)
{
	Cache<std::string, std::string> cache(2, timeProvider_);
	cache.Put("key1", "value1");
	EXPECT_TRUE(cache.Contains("key1"));

	auto value = cache.Get("key1");
	EXPECT_FALSE(value == "value");
}

TEST_F(CacheTest, PutOneItemContainsFalseGetFalse)
{
	Cache<std::string, std::string> cache(2, timeProvider_);
	cache.Put("key1", "value1");
	EXPECT_FALSE(cache.Contains("key2"));

	auto value = cache.Get("key1");
	EXPECT_FALSE(value == "alue1");
}

TEST_F(CacheTest, GivenEmptyCache_WhenGetAndContainsWithKeyAreCalled_ThenGetShouldThrowAndContainsShouldReturnFalse)
{
	Cache<int, std::string> cache(2, timeProvider_);
	EXPECT_THROW(cache.Get(42), std::out_of_range);
	EXPECT_FALSE(cache.Contains(42));
}

TEST_F(CacheTest, Test)
{
	Cache<int, std::string> cache(3, timeProvider_);
	cache.Put(1, "one", 1);
	cache.Put(2, "two", 2000);
}

TEST_F(CacheTest, WhenTickReturnsFalse_ItemsAreNotRemoved)
{
	auto now = std::chrono::steady_clock::now();
	auto future = now + std::chrono::milliseconds(5000);
	bool exit = false;

	EXPECT_CALL(timeProvider_, Now())
		.WillRepeatedly(::testing::Return(future));

	EXPECT_CALL(timeProvider_, Tick()).WillRepeatedly([&] {
		exit = true;
		return false;
	});

	Cache<int, std::string> cache(5, timeProvider_);

	cache.Put(1, "one", 1000);

	while (exit == false);

	EXPECT_TRUE(cache.Contains(1));
}

TEST_F(CacheTest, WhenMultipleItemsExpire_AllAreRemoved)
{
		auto now = std::chrono::steady_clock::now();
		auto expired = now + std::chrono::milliseconds(3000);
		std::atomic<bool> cacheFulfilled = false;
		std::atomic<bool> exit = false;
		int cnt = 0;
		Cache<int, std::string> cache(5, timeProvider_);

		ON_CALL(timeProvider_, Tick()).WillByDefault([&cacheFulfilled, &cnt, &exit] {
			if (cacheFulfilled)
			{
				if (cnt > 0)
					exit = true;

				cnt++;
				return true;
			}
			else
			{
				return false;
			}
		});

		EXPECT_CALL(timeProvider_, Now())
			.WillOnce(::testing::Return(now))
			.WillOnce(::testing::Return(now))
			.WillOnce(::testing::Return(now))
			.WillRepeatedly(::testing::Return(expired));



		cache.Put(1, "one", 1000);
		cache.Put(2, "two", 1000);
		cache.Put(3, "three", 1000);
		cacheFulfilled = true;

		while (exit == false);

		if (cache.Contains(1) || cache.Contains(2) || cache.Contains(3))
		{
			std::cout << "Cache still contains items after expiration." << std::endl;
		}

		EXPECT_FALSE(cache.Contains(1));
		EXPECT_FALSE(cache.Contains(2));
		EXPECT_FALSE(cache.Contains(3));
}

TEST_F(CacheTest, WhenCacheExceedsMaxSize_OldestItemIsRemoved)
{
	auto now = std::chrono::steady_clock::now();

	EXPECT_CALL(timeProvider_, Now())
		.WillRepeatedly(::testing::Return(now));

	Cache<int, std::string> cache(2, timeProvider_);

	cache.Put(1, "one");
	cache.Put(2, "two");
	cache.Put(3, "three");

	EXPECT_FALSE(cache.Contains(1));
	EXPECT_TRUE(cache.Contains(2));
	EXPECT_TRUE(cache.Contains(3));
}

TEST_F(CacheTest, OldestItemShouldGetUpdatedWhenGetCalled)
{
	auto now = std::chrono::steady_clock::now();

	EXPECT_CALL(timeProvider_, Now())
		.WillRepeatedly(::testing::Return(now));

	Cache<int, std::string> cache(2, timeProvider_);

	cache.Put(1, "one");
	cache.Put(2, "two");

	cache.Get(1);

	cache.Put(3, "three");

	EXPECT_TRUE(cache.Contains(1));
	EXPECT_FALSE(cache.Contains(2));
	EXPECT_TRUE(cache.Contains(3));
}
