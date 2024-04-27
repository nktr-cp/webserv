#include "Config.hpp"
#include <gtest/gtest.h>

TEST(ConfigTest, hoge) {
	Config config;

	config.parseConfig("tests/configs/test1.conf");
	EXPECT_EQ(config.servers_.size(), 1);
	EXPECT_EQ(config.servers_[0].getPort(), "8081");
	EXPECT_EQ(config.servers_[0].getServerName(), "test_server");
	EXPECT_EQ(config.servers_[0].locations_.size(), 1);
	EXPECT_EQ(config.servers_[0].locations_[0].getUri(), "/");
	EXPECT_EQ(config.servers_[0].locations_[0].getRoot(), "/docs/www/");
	EXPECT_EQ(config.servers_[0].locations_[0].getIndex(), "default.html");
}

int main(int argc, char** argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TEST;
}
