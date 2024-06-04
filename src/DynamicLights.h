#pragma once
#include "Global.h"

class DynamicLightsManager {
public:
    struct LightInfo {
        BlockPos      mPos;
        DimensionType mDimId;
        uint8_t       mLightLevel;
    };

private:
    std::unordered_map<std::string, uint8_t> mItemLightInfo;
    std::unordered_map<int64_t, LightInfo>   mRuntimeLightMap;
    std::unordered_map<mce::UUID, bool>      mPlayerConfig;

public:
    DynamicLightsManager();
    ~DynamicLightsManager();

public:
    uint8_t getItemLightLevel(ItemStack const& item);

    void readConfig();

    void setItemLightInfo(std::string const& typeName, uint8_t level);

    bool deleteItemLightInfo(std::string const& typeName);

    void onPlayerTick(Player& player);

    void onItemActorTick(ItemActor& itemActor);

    void sendPacket(Packet& packet, DimensionType dimId);

    void lightOn(BlockPos const& pos, DimensionType dimId, uint8_t lightLevel);

    void lightOff(BlockPos const& pos, DimensionType dimId);

    void remove(int64_t identifider);

    void loadPlayerConfig();

    void savePlayerConfig();

    bool getPlayerConfig(mce::UUID const& uuid);

    void setPlayerConfig(mce::UUID const& uuid, bool value);
};
