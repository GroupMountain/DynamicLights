#include "DynamicLights.h"
#include "Entry.h"

using namespace ll::chrono_literals;

struct ItemActorTickHook;
struct PlayerTickHook;
struct PlayerDestructHook;
struct ItemActorDestructHook;

DynamicLightsManager::DynamicLightsManager() {
    readConfig();
    loadPlayerConfig();
    ll::memory::HookRegistrar<PlayerTickHook, ItemActorTickHook, PlayerDestructHook, ItemActorDestructHook>().hook();
}

DynamicLightsManager::~DynamicLightsManager() {
    savePlayerConfig();
    for (auto& [itemActorId, info] : mRuntimeLightMap) {
        lightOff(info.mPos, info.mDimId);
    }
    ll::memory::HookRegistrar<PlayerTickHook, ItemActorTickHook, PlayerDestructHook, ItemActorDestructHook>().unhook();
}

void DynamicLightsManager::readConfig() {
    mItemLightInfo = DynamicLights::Entry::getInstance().getConfig().itemLightInfo;
}

uint8_t DynamicLightsManager::getItemLightLevel(ItemStack const& item) {
    auto const& name = item.getTypeName();
    if (mItemLightInfo.contains(name)) {
        return mItemLightInfo[name];
    }
    return 0;
}

void DynamicLightsManager::onPlayerTick(Player& player) {
    auto&       mainhandItem = player.getCarriedItem();
    auto&       offhandItem  = player.getOffhandSlot();
    auto const& lightLevel   = std::max(getItemLightLevel(mainhandItem), getItemLightLevel(offhandItem));
    auto const& dimId        = player.getDimensionId();
    auto        pos          = BlockPos(player.getPosition());
    auto&       lastInfo     = mRuntimeLightMap[player.getOrCreateUniqueID().rawID];
    if (lightLevel > 0) {
        if (lastInfo.mPos != pos || lastInfo.mDimId != dimId) {
            lightOff(lastInfo.mPos, lastInfo.mDimId);
            lightOn(pos, dimId, lightLevel);
        } else {
            if (lastInfo.mLightLevel != lightLevel) {
                lightOn(pos, dimId, lightLevel);
            }
        }
    } else {
        lightOff(lastInfo.mPos, lastInfo.mDimId);
    }
    lastInfo = LightInfo{pos, dimId, lightLevel};
}

void DynamicLightsManager::onItemActorTick(ItemActor& itemActor) {
    auto&       item        = itemActor.item();
    auto const& lightLevel  = getItemLightLevel(item);
    auto const& dimId       = itemActor.getDimensionId();
    auto        pos         = BlockPos(itemActor.getPosition());
    auto        itemActorId = reinterpret_cast<int64_t>(&itemActor); // 使用 ItemActor 的地址作为键值
    auto&       lastInfo    = mRuntimeLightMap[itemActorId];

    if (lightLevel > 0) {
        if (lastInfo.mPos != pos || lastInfo.mDimId != dimId) {
            lightOff(lastInfo.mPos, lastInfo.mDimId);
            lightOn(pos, dimId, lightLevel);
        } else {
            if (lastInfo.mLightLevel != lightLevel) {
                lightOn(pos, dimId, lightLevel);
            }
        }
    } else {
        lightOff(lastInfo.mPos, lastInfo.mDimId);
    }
    lastInfo = LightInfo{pos, dimId, lightLevel};
}

void DynamicLightsManager::sendPacket(Packet& packet, DimensionType dimId) {
    auto level = GMLIB_Level::getInstance();
    level->getOrCreateDimension(dimId)->forEachPlayer([&](Player& pl) -> bool {
        if (getPlayerConfig(pl.getUuid())) {
            level->sendPacketTo(packet, pl);
        }
        return true;
    });
}

void DynamicLightsManager::sendPacket(Packet& packet, Player& pl) {
    GMLIB_Level::getInstance()->sendPacketTo(packet, pl);
}

void DynamicLightsManager::lightOn(BlockPos const& pos, DimensionType dimId, uint8_t lightLevel) {
    auto& originBlock = GMLIB_Level::getInstance()->getBlock(pos, dimId);
    auto  fakeBlock   = Block::tryGetFromRegistry("minecraft:light_block", lightLevel);
    auto  runtimeId   = fakeBlock->getRuntimeId();
    if (originBlock.isAir()) {
        UpdateBlockPacket pkt(pos, 1, runtimeId, 3);
        sendPacket(pkt, dimId);
    } else if (originBlock.getTypeName() == "minecraft:water" || originBlock.getTypeName() == "minecraft:flowing_water"
               || originBlock.getTypeName() == "minecraft:snow_layer") {
        UpdateBlockPacket pkt(pos, 0, runtimeId, 3);
        sendPacket(pkt, dimId);
    }
}

void DynamicLightsManager::lightOn(BlockPos const& pos, uint8_t lightLevel, Player& pl) {
    auto& originBlock = GMLIB_Level::getInstance()->getBlock(pos, pl.getDimensionId());
    auto  fakeBlock   = Block::tryGetFromRegistry("minecraft:light_block", lightLevel);
    auto  runtimeId   = fakeBlock->getRuntimeId();
    if (originBlock.isAir()) {
        UpdateBlockPacket pkt(pos, 1, runtimeId, 3);
        sendPacket(pkt, pl);
    } else if (originBlock.getTypeName() == "minecraft:water" || originBlock.getTypeName() == "minecraft:flowing_water"
               || originBlock.getTypeName() == "minecraft:snow_layer") {
        UpdateBlockPacket pkt(pos, 0, runtimeId, 3);
        sendPacket(pkt, pl);
    }
}

void DynamicLightsManager::lightOff(BlockPos const& pos, DimensionType dimId) {
    auto& originBlock = GMLIB_Level::getInstance()->getBlock(pos, dimId);
    auto  runtimeId   = originBlock.getRuntimeId();
    if (originBlock.getTypeName() == "minecraft:water" || originBlock.getTypeName() == "minecraft:flowing_water"
        || originBlock.getTypeName() == "minecraft:snow_layer") {
        UpdateBlockPacket pkt(pos, 0, runtimeId, 3);
        sendPacket(pkt, dimId);
    } else {
        UpdateBlockPacket pkt(pos, 1, runtimeId, 3);
        sendPacket(pkt, dimId);
    }
}

void DynamicLightsManager::lightOff(BlockPos const& pos, Player& pl) {
    auto& originBlock = GMLIB_Level::getInstance()->getBlock(pos, pl.getDimensionId());
    auto  runtimeId   = originBlock.getRuntimeId();
    if (originBlock.getTypeName() == "minecraft:water" || originBlock.getTypeName() == "minecraft:flowing_water"
        || originBlock.getTypeName() == "minecraft:snow_layer") {
        UpdateBlockPacket pkt(pos, 0, runtimeId, 3);
        sendPacket(pkt, pl);
    } else {
        UpdateBlockPacket pkt(pos, 1, runtimeId, 3);
        sendPacket(pkt, pl);
    }
}

void DynamicLightsManager::setItemLightInfo(std::string const& typeName, uint8_t level) {
    mItemLightInfo[typeName]                                      = level;
    DynamicLights::Entry::getInstance().getConfig().itemLightInfo = mItemLightInfo;
    DynamicLights::Entry::getInstance().saveConfig();
}

bool DynamicLightsManager::deleteItemLightInfo(std::string const& typeName) {
    if (mItemLightInfo.contains(typeName)) {
        mItemLightInfo.erase(typeName);
        DynamicLights::Entry::getInstance().getConfig().itemLightInfo = mItemLightInfo;
        return DynamicLights::Entry::getInstance().saveConfig();
    }
    return false;
}

void DynamicLightsManager::loadPlayerConfig() {
    auto json = GMLIB::Files::JsonFile::initJson(
        DynamicLights::Entry::getInstance().getSelf().getDataDir() / u8"PlayerData.json",
        nlohmann::json::object()
    );
    for (nlohmann::json::const_iterator it = json.begin(); it != json.end(); ++it) {
        if (it.value().is_boolean()) {
            auto uuid           = mce::UUID::fromString(it.key());
            mPlayerConfig[uuid] = it.value().get<bool>();
        }
    }
}

void DynamicLightsManager::savePlayerConfig() {
    auto json = nlohmann::json::object();
    for (auto& [key, val] : mPlayerConfig) {
        json[key.asString()] = val;
    }
    GMLIB::Files::JsonFile::writeFile(
        DynamicLights::Entry::getInstance().getSelf().getDataDir() / u8"PlayerData.json",
        json
    );
}

bool DynamicLightsManager::getPlayerConfig(mce::UUID const& uuid) {
    if (!mPlayerConfig.contains(uuid)) {
        setPlayerConfig(uuid, true);
    }
    return mPlayerConfig[uuid];
}

void DynamicLightsManager::setPlayerConfig(mce::UUID const& uuid, bool value) {
    mPlayerConfig[uuid] = value;
    savePlayerConfig();
}

void DynamicLightsManager::remove(int64_t itemActorId) {
    auto it = mRuntimeLightMap.find(itemActorId);
    if (it != mRuntimeLightMap.end()) {
        auto& info = it->second;
        lightOff(info.mPos, info.mDimId);
        mRuntimeLightMap.erase(it);
    }
}

void DynamicLightsManager::removeLightsFrom(Player& pl) {
    for (auto& [id, info] : mRuntimeLightMap) {
        if (pl.getDimensionId() == info.mDimId) {
            lightOff(info.mPos, pl);
        }
    }
}

void DynamicLightsManager::sendLightsTo(Player& pl) {
    for (auto& [id, info] : mRuntimeLightMap) {
        if (pl.getDimensionId() == info.mDimId) {
            lightOn(info.mPos, info.mLightLevel, pl);
        }
    }
}

LL_TYPE_INSTANCE_HOOK(PlayerTickHook, ll::memory::HookPriority::Normal, Player, &Player::$normalTick, void) {
    DynamicLights::Entry::getInstance().getLightsManager().onPlayerTick(*this);
    return origin();
}

LL_TYPE_INSTANCE_HOOK(
    ItemActorTickHook,
    ll::memory::HookPriority::Normal,
    ItemActor,
    &ItemActor::postNormalTick,
    void
) {
    DynamicLights::Entry::getInstance().getLightsManager().onItemActorTick(*this);
    return origin();
}

LL_TYPE_INSTANCE_HOOK(
    PlayerDestructHook,
    ll::memory::HookPriority::Normal,
    ServerPlayer,
    &ServerPlayer::disconnect,
    void
) {
    DynamicLights::Entry::getInstance().getLightsManager().remove(this->getOrCreateUniqueID().rawID);
    return origin();
}

LL_TYPE_INSTANCE_HOOK(
    ItemActorDestructHook,
    ll::memory::HookPriority::Normal,
    ItemActor,
    &ItemActor::$dtor,
    void,
    void
) {
    DynamicLights::Entry::getInstance().getLightsManager().remove((uintptr_t)this);
    return origin();
}