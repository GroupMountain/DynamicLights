#pragma once
#include <iostream>
#include <unordered_map>
#include <unordered_set>

namespace DynamicLights {

struct Config {
    int version = 1;

    std::string language = "zh_CN";

    std::unordered_set<std::string> offhandItems = {"minecraft:torch"};

    std::unordered_map<std::string, uint8_t> itemLightInfo = {
        {"minecraft:torch", 14}
    };
};
} // namespace DynamicLights