#include "Entry.h"
#include "Global.h"

struct ManageCommand {
    enum class Action { add, remove } action;
    CommandItem item;
    int         lightLevel;
};

void registerAdminCommand() {
    auto& cmd = ll::command::CommandRegistrar::getInstance().getOrCreateCommand(
        "dynamiclightsmanager",
        tr("command.dynamiclightsmanager.desc"),
        CommandPermissionLevel::GameDirectors
    );
    cmd.alias("dlm");
    cmd.overload<ManageCommand>()
        .text("items")
        .text("set")
        .required("item")
        .required("lightLevel")
        .execute<[](CommandOrigin const& origin, CommandOutput& output, ManageCommand const& param) {
            if (auto item = param.item.createInstance(1, 0, output, false)) {
                DynamicLights::Entry::getInstance()->getLightsManager().setItemLightInfo(
                    item->getTypeName(),
                    param.lightLevel
                );
                return output.success(
                    tr("command.dynamiclightsmanager.setItem", {item->getDescriptionName(), S(param.lightLevel)})
                );
            }
            return output.error(tr("command.dynamiclightsmanager.invalidItem"));
        }>();
    cmd.overload<ManageCommand>()
        .text("items")
        .text("delete")
        .required("item")
        .execute<[](CommandOrigin const& origin, CommandOutput& output, ManageCommand const& param) {
            if (auto item = param.item.createInstance(1, 0, output, false)) {
                auto result =
                    DynamicLights::Entry::getInstance()->getLightsManager().deleteItemLightInfo(item->getTypeName());
                return result ? output.success(
                                    tr("command.dynamiclightsmanager.deleteItem.success", {item->getDescriptionName()})
                                )
                              : output.error(
                                    tr("command.dynamiclightsmanager.deleteItem.failed", {item->getDescriptionName()})
                                );
            }
            return output.error(tr("command.dynamiclightsmanager.invalidItem"));
        }>();
    cmd.overload<ManageCommand>()
        .text("offhand")
        .required("action")
        .required("item")
        .execute<[](CommandOrigin const& origin, CommandOutput& output, ManageCommand const& param) {
            if (auto item = param.item.createInstance(1, 0, output, false)) {
                auto& itemList = DynamicLights::Entry::getInstance()->getConfig().offhandItems;
                auto  typeName = item->getTypeName();
                if (param.action == ManageCommand::Action::add) {
                    if (!itemList.contains(typeName)) {
                        itemList.insert(typeName);
                        DynamicLights::Entry::getInstance()->saveConfig();
                        return output.success(
                            tr("command.dynamiclightsmanager.offhand.add.success", {item->getDescriptionName()})
                        );
                    }
                    return output.error(
                        tr("command.dynamiclightsmanager.offhand.add.failed", {item->getDescriptionName()})
                    );
                } else {
                    if (itemList.contains(typeName)) {
                        itemList.erase(typeName);
                        DynamicLights::Entry::getInstance()->saveConfig();
                        return output.success(
                            tr("command.dynamiclightsmanager.offhand.remove.success", {item->getDescriptionName()})
                        );
                    }
                    return output.error(
                        tr("command.dynamiclightsmanager.offhand.remove.failed", {item->getDescriptionName()})
                    );
                }
            }
            return output.error(tr("command.dynamiclightsmanager.invalidItem"));
        }>();
    cmd.overload<ManageCommand>()
        .text("reload")
        .execute<[](CommandOrigin const& origin, CommandOutput& output, ManageCommand const& param) {
            DynamicLights::Entry::getInstance()->loadConfig();
            DynamicLights::Entry::getInstance()->getI18n().loadAllLanguages();
            DynamicLights::Entry::getInstance()->getLightsManager().readConfig();
            return output.success(tr("command.dynamiclightsmanager.reload"));
        }>();
}

void registerPlayerCommand() {
    auto& cmd = ll::command::CommandRegistrar::getInstance().getOrCreateCommand(
        "dynamiclights",
        tr("command.dynamiclights.desc"),
        CommandPermissionLevel::Any
    );
    cmd.alias("dl");
    cmd.overload<ManageCommand>()
        .execute<[](CommandOrigin const& origin, CommandOutput& output, ManageCommand const& param) {
            if (origin.getOriginType() == CommandOriginType::Player) {
                auto  player  = (Player*)origin.getEntity();
                auto& manager = DynamicLights::Entry::getInstance()->getLightsManager();
                auto  enable  = !manager.getPlayerConfig(player->getUuid());
                manager.setPlayerConfig(player->getUuid(), enable);
                return enable ? output.success(tr("command.dynamiclights.enabled"))
                              : output.success(tr("command.dynamiclights.disabled"));
            }
            return output.error(tr("command.dynamiclights.invalidCommandOrigin"));
        }>();
}

void registerCommands() {
    registerAdminCommand();
    registerPlayerCommand();
}