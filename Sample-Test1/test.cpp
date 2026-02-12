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
	TimeProviderMock timeProvider_;
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

	EXPECT_CALL(timeProvider_, Tick())
		.WillRepeatedly(::testing::Return(false));

	EXPECT_CALL(timeProvider_, Now())
		.WillRepeatedly(::testing::Return(future));

	Cache<int, std::string> cache(5, timeProvider_);

	cache.Put(1, "one", 1000);

	std::this_thread::sleep_for(std::chrono::milliseconds(50));

	EXPECT_TRUE(cache.Contains(1));
}

TEST_F(CacheTest, WhenMultipleItemsExpire_AllAreRemoved)
{
	auto now = std::chrono::steady_clock::now();
	auto expired = now + std::chrono::milliseconds(3000);

	EXPECT_CALL(timeProvider_, Tick())
		.WillRepeatedly(::testing::Return(true));

	EXPECT_CALL(timeProvider_, Now())
		.WillOnce(::testing::Return(now))
		.WillOnce(::testing::Return(now))
		.WillOnce(::testing::Return(now))
		.WillRepeatedly(::testing::Return(expired));

	Cache<int, std::string> cache(5, timeProvider_);

	cache.Put(1, "one", 1000);
	cache.Put(2, "two", 1000);
	cache.Put(3, "three", 1000);

	std::this_thread::sleep_for(std::chrono::milliseconds(50));

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
