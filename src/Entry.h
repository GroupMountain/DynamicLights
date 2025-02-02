#pragma once
#include "Config.h"
#include "DynamicLights.h"
#include "Global.h"

namespace DynamicLights {

using namespace GMLIB::Files::I18n;

class Entry {

public:
    static Entry& getInstance();

    Entry() : mSelf(*ll::mod::NativeMod::current()) {}

    [[nodiscard]] ll::mod::NativeMod& getSelf() const { return mSelf; }

    /// @return True if the plugin is loaded successfully.
    bool load();

    /// @return True if the plugin is enabled successfully.
    bool enable();

    /// @return True if the plugin is disabled successfully.
    bool disable();

    /// @return True if the plugin is unloaded successfully.
    bool unload();

    Config& getConfig();

    bool loadConfig();

    bool saveConfig();

    LangI18n& getI18n();

    DynamicLightsManager& getLightsManager();

private:
    ll::mod::NativeMod&                 mSelf;
    std::optional<Config>               mConfig;
    std::optional<LangI18n>             mI18n;
    std::optional<DynamicLightsManager> mManager;
};

} // namespace DynamicLights