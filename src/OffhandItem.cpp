#include "Entry.h"
#include "Global.h"

void listenSwitchItem() {
    ll::event::EventBus::getInstance().emplaceListener<ll::event::PlayerUseItemEvent>(
        [](ll::event::PlayerUseItemEvent& ev) {
            auto& config   = DynamicLights::Entry::getInstance()->getConfig();
            auto& item     = ev.item();
            auto  mainhand = item.clone();
            if (config.offhandItems.contains(item.getTypeName())) {
                auto offhand = ev.self().getOffhandSlot().clone();
                ev.self().setOffhandSlot(mainhand);
                ev.self().setCarriedItem(offhand);
                ev.self().refreshInventory();
            }
        }
    );
}