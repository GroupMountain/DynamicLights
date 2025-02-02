#include "Entry.h"
#include "Global.h"

void listenSwitchItem() {
    ll::event::EventBus::getInstance().emplaceListener<ll::event::PlayerUseItemEvent>(
        [](ll::event::PlayerUseItemEvent& ev) {
            auto& config   = DynamicLights::Entry::getInstance().getConfig();
            auto  mainhand = ev.item().clone();
            auto  offhand  = ev.self().getOffhandSlot().clone();
            if (config.offhandItems.contains(mainhand.getTypeName()) && offhand.isNull()) {
                ev.self().setOffhandSlot(mainhand);
                ev.self().setCarriedItem(offhand);
                ev.self().refreshInventory();
            }
        }
    );
}