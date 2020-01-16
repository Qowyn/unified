#include "Events/StoreEvents.hpp"
#include "API/CNWSCreature.hpp"
#include "API/CNWSItem.hpp"
#include "API/CNWSStore.hpp"
#include "API/Functions.hpp"
#include "Utils.hpp"
#include "Events.hpp"

namespace Events {

using namespace NWNXLib;
using namespace NWNXLib::API;

static NWNXLib::Hooking::FunctionHook* m_RequestBuyHook = nullptr;
static NWNXLib::Hooking::FunctionHook* m_RequestSellHook = nullptr;
static NWNXLib::Hooking::FunctionHook* m_AcquireItemHook = nullptr;
static NWNXLib::Hooking::FunctionHook* m_SellItemHook = nullptr;

StoreEvents::StoreEvents(Services::HooksProxy* hooker)
{
    Events::InitOnFirstSubscribe("NWNX_ON_REQUEST_BUY_.*", [hooker]() {
        hooker->RequestExclusiveHook<API::Functions::_ZN12CNWSCreature10RequestBuyEjjj>(&RequestBuyHook);
        m_RequestBuyHook = hooker->FindHookByAddress(API::Functions::_ZN12CNWSCreature10RequestBuyEjjj);
    });

    Events::InitOnFirstSubscribe("NWNX_ON_REQUEST_SELL_.*", [hooker]() {
        hooker->RequestExclusiveHook<API::Functions::_ZN12CNWSCreature11RequestSellEjj>(&RequestSellHook);
        m_RequestSellHook = hooker->FindHookByAddress(API::Functions::_ZN12CNWSCreature11RequestSellEjj);
    });

    Events::InitOnFirstSubscribe("NWNX_ON_STORE_ACQUIRE_ITEM_.*", [hooker]() {
        hooker->RequestExclusiveHook<API::Functions::_ZN9CNWSStore11AcquireItemEP8CNWSItemihh, int32_t>(&AcquireItemHook);
        m_AcquireItemHook = hooker->FindHookByAddress(API::Functions::_ZN9CNWSStore11AcquireItemEP8CNWSItemihh);
    });

    Events::InitOnFirstSubscribe("NWNX_ON_STORE_SELL_ITEM_.*", [hooker]() {
        hooker->RequestExclusiveHook<API::Functions::_ZN9CNWSStore8SellItemEP8CNWSItemP12CNWSCreaturehh, int32_t>(&SellItemHook);
        m_SellItemHook = hooker->FindHookByAddress(API::Functions::_ZN9CNWSStore8SellItemEP8CNWSItemP12CNWSCreaturehh);
    });
}

BOOL StoreEvents::RequestBuyHook(CNWSCreature* thisPtr, OBJECT_ID oidItemToBuy, OBJECT_ID oidStore, OBJECT_ID oidDesiredRepository)
{
    BOOL retVal;
    std::string sBeforeEventResult;
    std::string sAfterEventResult;

    Events::PushEventData("ITEM", Utils::ObjectIDToString(oidItemToBuy));
    Events::PushEventData("STORE", Utils::ObjectIDToString(oidStore));
    Events::PushEventData("CONTAINER", Utils::ObjectIDToString(oidDesiredRepository));

    retVal = Events::SignalEvent("NWNX_ON_REQUEST_BUY_BEFORE", thisPtr->m_idSelf, &sBeforeEventResult)
             ? m_RequestBuyHook->CallOriginal<BOOL>(thisPtr, oidItemToBuy, oidStore, oidDesiredRepository) : sBeforeEventResult == "1";

    Events::PushEventData("ITEM", Utils::ObjectIDToString(oidItemToBuy));
    Events::PushEventData("STORE", Utils::ObjectIDToString(oidStore));
    Events::PushEventData("CONTAINER", Utils::ObjectIDToString(oidDesiredRepository));
    Events::PushEventData("BEFORE_RESULT", std::to_string(retVal));

    Events::SignalEvent("NWNX_ON_REQUEST_BUY_AFTER", thisPtr->m_idSelf, &sAfterEventResult);

    retVal = sAfterEventResult.empty() ? retVal : sAfterEventResult == "1";

    return retVal;
}

BOOL StoreEvents::RequestSellHook(CNWSCreature* thisPtr, OBJECT_ID oidItemToSell, OBJECT_ID oidStore)
{
    BOOL retVal;
    std::string sBeforeEventResult;
    std::string sAfterEventResult;

    Events::PushEventData("ITEM", Utils::ObjectIDToString(oidItemToSell));
    Events::PushEventData("STORE", Utils::ObjectIDToString(oidStore));

    retVal = Events::SignalEvent("NWNX_ON_REQUEST_SELL_BEFORE", thisPtr->m_idSelf, &sBeforeEventResult)
             ? m_RequestSellHook->CallOriginal<BOOL>(thisPtr, oidItemToSell, oidStore) : sBeforeEventResult == "1";

    Events::PushEventData("ITEM", Utils::ObjectIDToString(oidItemToSell));
    Events::PushEventData("STORE", Utils::ObjectIDToString(oidStore));
    Events::PushEventData("BEFORE_RESULT", std::to_string(retVal));

    Events::SignalEvent("NWNX_ON_REQUEST_SELL_AFTER", thisPtr->m_idSelf, &sAfterEventResult);

    retVal = sAfterEventResult.empty() ? retVal : sAfterEventResult == "1";

    return retVal;
}

BOOL StoreEvents::AcquireItemHook(CNWSStore* thisPtr, CNWSItem* pItem, BOOL bInfinite, uint8_t x, uint8_t y)
{
    BOOL retVal;
    std::string sBeforeEventResult;
    std::string sAfterEventResult;

    Events::PushEventData("ITEM", Utils::ObjectIDToString(pItem->m_idSelf));
    Events::PushEventData("INFINITE", std::to_string(bInfinite));
    Events::PushEventData("X", std::to_string(x));
    Events::PushEventData("Y", std::to_string(y));

    retVal = Events::SignalEvent("NWNX_ON_STORE_ACQUIRE_ITEM_BEFORE", thisPtr->m_idSelf, &sBeforeEventResult)
             ? m_AcquireItemHook->CallOriginal<BOOL>(thisPtr, pItem, bInfinite, x, y) : sBeforeEventResult == "1";

    Events::PushEventData("ITEM", Utils::ObjectIDToString(pItem->m_idSelf));
    Events::PushEventData("INFINITE", std::to_string(bInfinite));
    Events::PushEventData("X", std::to_string(x));
    Events::PushEventData("Y", std::to_string(y));
    Events::PushEventData("BEFORE_RESULT", std::to_string(retVal));

    Events::SignalEvent("NWNX_ON_STORE_ACQUIRE_ITEM_AFTER", thisPtr->m_idSelf, &sAfterEventResult);

    retVal = sAfterEventResult.empty() ? retVal : sAfterEventResult == "1";

    return retVal;
}

BOOL StoreEvents::SellItemHook(CNWSStore* thisPtr, CNWSItem * pItem, CNWSCreature * pCreature, uint8_t x, uint8_t y)
{
    BOOL retVal;
    std::string sBeforeEventResult;
    std::string sAfterEventResult;

    Events::PushEventData("ITEM", Utils::ObjectIDToString(pItem->m_idSelf));
    Events::PushEventData("BUYER", Utils::ObjectIDToString(pCreature->m_idSelf));
    Events::PushEventData("X", std::to_string(x));
    Events::PushEventData("Y", std::to_string(y));

    retVal = Events::SignalEvent("NWNX_ON_STORE_SELL_ITEM_BEFORE", thisPtr->m_idSelf, &sBeforeEventResult)
             ? m_SellItemHook->CallOriginal<BOOL>(thisPtr, pItem, pCreature, x, y) : sBeforeEventResult == "1";

    Events::PushEventData("ITEM", Utils::ObjectIDToString(pItem->m_idSelf));
    Events::PushEventData("BUYER", Utils::ObjectIDToString(pCreature->m_idSelf));
    Events::PushEventData("X", std::to_string(x));
    Events::PushEventData("Y", std::to_string(y));
    Events::PushEventData("BEFORE_RESULT", std::to_string(retVal));

    Events::SignalEvent("NWNX_ON_STORE_SELL_ITEM_AFTER", thisPtr->m_idSelf, &sAfterEventResult);

    retVal = sAfterEventResult.empty() ? retVal : sAfterEventResult == "1";

    return retVal;
}

}
