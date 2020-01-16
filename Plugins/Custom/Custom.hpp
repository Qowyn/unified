#pragma once

#include "Plugin.hpp"
#include "Services/Events/Events.hpp"
#include "API/CExoLinkedListInternal.hpp"
#include "API/CGameEffect.hpp"
#include "API/CNWSArea.hpp"
#include "API/CNWSAreaOfEffectObject.hpp"
#include "API/CNWSCombatRound.hpp"
#include "API/CNWSCreature.hpp"
#include "API/CNWSCreatureStats.hpp"
#include "API/CNWSDoor.hpp"
#include "API/CNWSEffectListHandler.hpp"
#include "API/CNWSEncounter.hpp"
#include "API/CNWSMessage.hpp"
#include "API/CNWSObject.hpp"
#include "API/CNWSPlaceable.hpp"
#include "API/CNWSPlayer.hpp"
#include "API/CNWSTrigger.hpp"
#include "API/CServerExoApp.hpp"
#include "API/CServerExoAppInternal.hpp"
#include "API/CWorldTimer.hpp"
#include "API/Types.hpp"

using ArgumentStack = NWNXLib::Services::Events::ArgumentStack;

namespace Custom {

class Custom : public NWNXLib::Plugin
{
public:
    Custom(const Plugin::CreateParams& params);
    virtual ~Custom();

private:
    ArgumentStack PossessCreature(ArgumentStack&& args);
    ArgumentStack UnpossessCreature(ArgumentStack&& args);
    ArgumentStack GetFactionCount(ArgumentStack&& args);
    ArgumentStack GetFactionId(ArgumentStack&& args);
    ArgumentStack SetFactionId(ArgumentStack&& args);
    ArgumentStack GetFactionName(ArgumentStack&& args);
    ArgumentStack GetSuppressDialog(ArgumentStack&& args);
    ArgumentStack SetSuppressDialog(ArgumentStack&& args);
    ArgumentStack SetHasInventory(ArgumentStack&& args);
    ArgumentStack GetStoreBlackMarket(ArgumentStack&& args);
    ArgumentStack GetStoreBlackMarketMarkDown(ArgumentStack&& args);
    ArgumentStack GetStoreMarkDown(ArgumentStack&& args);
    ArgumentStack GetStoreMarkUp(ArgumentStack&& args);
    ArgumentStack CalculateItemBuyPrice(ArgumentStack&& args);
    ArgumentStack CalculateItemSellPrice(ArgumentStack&& args);
    ArgumentStack CompareItem(ArgumentStack&& args);
    CNWSPlayer *player(ArgumentStack& args);
    static void InitializeNumberOfAttacks(CNWSCombatRound*);
    static int32_t GetFavoredEnemyBonus(CNWSCreatureStats*, CNWSCreature*);
    static int GetLevelByClass(CNWSCreatureStats *, uint32_t);
    static int32_t CNWSEffectListHandler__OnApplyTimestop(CNWSEffectListHandler*, CNWSObject*, CGameEffect*, int32_t);
    static int32_t CNWSEffectListHandler__OnRemoveTimestop(CNWSEffectListHandler*, CNWSObject*, CGameEffect*);
    static void CNWSObject__SetArea(CNWSObject*, CNWSArea*);
    static void CNWSObject__SetAreaPost(CNWSObject*, uint32_t, uint32_t);
    static int32_t CNWSMessage__SendServerToPlayerModule_SetPauseState(CNWSMessage*, uint8_t, int32_t);
    static void SendPauseState(CNWSMessage*, int32_t, bool, uint32_t);
    static bool HasTimestop(CNWSObject*);
    static int32_t TimestopCount(CNWSObject*);
    static bool IsDM(CNWSCreature*);
    static int32_t CServerExoAppInternal__IsOnActiveExclusionList(CServerExoAppInternal*, uint32_t);
    static int32_t CNWSMessage__HandlePlayerToServerInputMessage(CNWSMessage*, CNWSPlayer*, uint8_t);
    static void CWorldTimer__PauseWorldTimer(CWorldTimer*);
    static void CWorldTimer__UnpauseWorldTimer(CWorldTimer*);
    static uint8_t CServerExoApp__GetActivePauseState(CServerExoApp*);
    static uint8_t CNWSCreatureStats__GetUnarmedDamageDice(CNWSCreatureStats*);
    static uint8_t CNWSCreatureStats__GetUnarmedDamageDie(CNWSCreatureStats*);
    static int32_t CNWSMessage__SendServerToPlayerParty_List(CNWSMessage*, uint32_t, int32_t, uint32_t*, uint8_t, uint32_t);
    static int32_t CNWSMessage__WriteGameObjUpdate_PartyAIState(CNWSMessage*, CNWSPlayer*);
    static CWorldTimer* CServerExoAppInternal__GetActiveTimer(CServerExoAppInternal*, uint32_t);
    static bool IsOnExclusionList(uint32_t, AurList<OBJECT_ID>*);
    static void InitTimer(uint32_t);
    static void ResetTimer(uint32_t);
    static int32_t SendServerToPlayerChatMultiLangMessage(CNWSMessage*, uint8_t, OBJECT_ID, CExoLocString, OBJECT_ID, uint8_t, uint32_t *, uint32_t, int32_t, const CResRef*, int32_t, OBJECT_ID);
    static int32_t SendServerToPlayerChatMultiLang_Helper(CNWSMessage*, uint32_t, OBJECT_ID, CExoLocString, OBJECT_ID, uint8_t, BOOL, const CResRef*, BOOL, OBJECT_ID);
};

}