Config.h
#pragma once
#include <string>

class Config {
public:
    static Config& Instance();

    // 加载：环境变量优先，若未提供则尝试从 filePath 加载（默认 "config.ini"）
    void Load(const std::string& filePath = "config.ini");

    // 配置项
    std::string mdAddress;
    std::string brokerId;
    std::string userId;
    std::string password;

    std::string dbHost;
    int dbPort;
    std::string dbUser;
    std::string dbPassword;
    std::string dbSchema;

private:
    Config();
    void loadDefaults();
    void loadFromEnv();
    void loadFromFile(const std::string& filePath);
    static std::string trim(const std::string& s);
};