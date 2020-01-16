#pragma once

#include "Common.hpp"
#include "Services/Hooks/Hooks.hpp"

namespace Events {

class StoreEvents
{
public:
    StoreEvents(NWNXLib::Services::HooksProxy* hooker);

private:
    static BOOL RequestBuyHook(CNWSCreature*, OBJECT_ID, OBJECT_ID, OBJECT_ID);
    static BOOL RequestSellHook(CNWSCreature*, OBJECT_ID, OBJECT_ID);
    static BOOL AcquireItemHook(CNWSStore*, CNWSItem*, BOOL, uint8_t, uint8_t);
    static BOOL SellItemHook(CNWSStore*, CNWSItem*, CNWSCreature*, uint8_t, uint8_t);
};

}
