#include <gtest/gtest.h>

#include "config.hpp"

class ConfigParserTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // 既存の config/default.conf ファイルを利用するための準備
    config_file_path_ = "../config/default.conf";
  }

  std::string config_file_path_;
};

TEST_F(ConfigParserTest, LoadConfig) {
  Config config(config_file_path_);

  // サーバ設定のチェック
  const std::vector<ServerConfig>& server_configs = config.getServerConfigs();
  ASSERT_EQ(server_configs.size(), 3);

  // 1番目のサーバ設定のチェック
  const ServerConfig& server1 = server_configs[0];
  EXPECT_EQ(server1.getPort(), "8080");
  EXPECT_EQ(server1.getServerName(), "default_server");

  // エラーページのチェック
  EXPECT_EQ(server1.getErrors().at(301), "https://42tokyo.jp");

  // location設定のチェック
  const Location& loc1 = server1.getLocations()[0];
  EXPECT_EQ(loc1.getMethods(), GET);
  EXPECT_EQ(loc1.getIndex(), "index.html");
  EXPECT_FALSE(loc1.isAutoIndex());

  // 他のlocationのチェックも同様に追加
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
