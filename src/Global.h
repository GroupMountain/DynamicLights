#pragma once
#include <include_all.h>

#define PLUGIN_NAME "DynamicLights"
#define S(x) std::to_string(x)

extern ll::Logger logger;

extern void listenSwitchItem();
extern void registerCommands();

extern std::string tr(std::string const& key, std::vector<std::string> const& params = {});