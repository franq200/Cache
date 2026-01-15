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

TEST(CacheTest, PutOneItemContainsTrueGetTrue)
{
	TimeProviderMock timeProvider_;
	Cache<std::string, std::string> cache(2, timeProvider_);
	cache.Put("key1", "value1");
	EXPECT_CALL(timeProvider_, Now());
	EXPECT_TRUE(cache.Contains("key1"));

	auto value = cache.Get("key1");
	EXPECT_EQ(value, "value1");
}/*

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
*/