#pragma once
#include <iostream>
#include <unordered_map>

struct Config {
    int version = 1;

    std::string language = "zh_CN";

    std::unordered_map<std::string, uint8_t> itemLightLevel = {
        {"minecraft:torch", 14}
    };
};
