#include "Cache.h"
#include <gtest/gtest.h>

TEST(CacheTest, PutOneItemContainsTrueGetTrue)
{
	Cache<std::string, std::string> cache(2);
	cache.Put("key1", "value1");
	EXPECT_TRUE(cache.Contains("key1"));

	auto value = cache.Get("key1");
	EXPECT_EQ(value, "value1");
}

TEST(CacheTest, PutOneItemContainsFalseGetTrue)
{
	Cache<std::string, std::string> cache(2);
	cache.Put("key1", "value1");
	EXPECT_FALSE(cache.Contains("key"));

	auto value = cache.Get("key1");
	EXPECT_EQ(value, "value1");
}

TEST(CacheTest, PutOneItemContainsTrueGetFalse)
{
	Cache<std::string, std::string> cache(2);
	cache.Put("key1", "value1");
	EXPECT_TRUE(cache.Contains("key1"));

	auto value = cache.Get("key1");
	EXPECT_FALSE(value == "value");
}

TEST(CacheTest, PutOneItemContainsFalseGetFalse)
{
	Cache<std::string, std::string> cache(2);
	cache.Put("key1", "value1");
	EXPECT_FALSE(cache.Contains("key2"));

	auto value = cache.Get("key1");
	EXPECT_FALSE(value == "alue1");
}

TEST(CacheTest, GivenEmptyCache_WhenGetAndContainsWithKeyAreCalled_ThenGetShouldThrowAndContainsShouldReturnTrue)
{
	Cache<int, std::string> cache(2);
	EXPECT_THROW(cache.Get(42), std::out_of_range);
	EXPECT_FALSE(cache.Contains(42));
}