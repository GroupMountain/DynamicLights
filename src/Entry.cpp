#include "Entry.h"
#include "Global.h"
#include "Language.h"

namespace DynamicLights {

Entry& Entry::getInstance() {
    static Entry instance;
    return instance;
}

bool Entry::load() { return true; }

bool Entry::enable() {
    mConfig.emplace();
    if (!loadConfig()) {
        saveConfig();
    }
    mI18n.emplace(getSelf().getLangDir(), getConfig().language);
    mI18n->updateOrCreateLanguage("zh_CN", zh_CN);
    mI18n->loadAllLanguages();
    mI18n->setDefaultLanguage("zh_CN");
    mManager.emplace();
    listenSwitchItem();
    registerCommands();
    return true;
}

bool Entry::disable() {
    mManager.reset();
    mI18n.reset();
    mConfig.reset();
    return true;
}
bool Entry::unload() {
    // getInstance().reset();
    return true;
}
Config& Entry::getConfig() { return mConfig.value(); }

bool Entry::loadConfig() { return ll::config::loadConfig(*mConfig, getSelf().getConfigDir() / u8"config.json"); }

bool Entry::saveConfig() { return ll::config::saveConfig(*mConfig, getSelf().getConfigDir() / u8"config.json"); }

LangI18n& Entry::getI18n() { return mI18n.value(); }

DynamicLightsManager& Entry::getLightsManager() { return mManager.value(); }

} // namespace DynamicLights

LL_REGISTER_MOD(DynamicLights::Entry, DynamicLights::Entry::getInstance());

std::string tr(std::string const& key, std::vector<std::string> const& params) {
    return DynamicLights::Entry::getInstance().getI18n().get(key, params);
}