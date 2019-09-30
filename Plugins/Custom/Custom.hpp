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
    NWNXLib::API::CNWSPlayer *player(ArgumentStack& args);
    static void InitializeNumberOfAttacks(NWNXLib::API::CNWSCombatRound*);
    static int32_t GetFavoredEnemyBonus(NWNXLib::API::CNWSCreatureStats*, NWNXLib::API::CNWSCreature*);
    static int GetLevelByClass(NWNXLib::API::CNWSCreatureStats *, uint32_t);
    static int32_t CNWSEffectListHandler__OnApplyTimestop(NWNXLib::API::CNWSEffectListHandler*, NWNXLib::API::CNWSObject*, NWNXLib::API::CGameEffect*, int32_t);
    static int32_t CNWSEffectListHandler__OnRemoveTimestop(NWNXLib::API::CNWSEffectListHandler*, NWNXLib::API::CNWSObject*, NWNXLib::API::CGameEffect*);
    static void CNWSObject__SetArea(NWNXLib::API::CNWSObject*, NWNXLib::API::CNWSArea*);
    static void CNWSObject__SetAreaPost(NWNXLib::API::CNWSObject*, uint32_t, uint32_t);
    static int32_t CNWSMessage__SendServerToPlayerModule_SetPauseState(NWNXLib::API::CNWSMessage*, uint8_t, int32_t);
    static void SendPauseState(NWNXLib::API::CNWSMessage*, int32_t, bool, uint32_t);
    static bool HasTimestop(NWNXLib::API::CNWSObject*);
    static int32_t TimestopCount(NWNXLib::API::CNWSObject*);
    static bool IsDM(NWNXLib::API::CNWSCreature*);
    static int32_t CServerExoAppInternal__IsOnActiveExclusionList(NWNXLib::API::CServerExoAppInternal*, uint32_t);
    static int32_t CNWSMessage__HandlePlayerToServerInputMessage(NWNXLib::API::CNWSMessage*, NWNXLib::API::CNWSPlayer*, uint8_t);
    static void CWorldTimer__PauseWorldTimer(NWNXLib::API::CWorldTimer*);
    static void CWorldTimer__UnpauseWorldTimer(NWNXLib::API::CWorldTimer*);
    static uint8_t CServerExoApp__GetActivePauseState(NWNXLib::API::CServerExoApp*);
    static uint8_t CNWSCreatureStats__GetUnarmedDamageDice(NWNXLib::API::CNWSCreatureStats*);
    static uint8_t CNWSCreatureStats__GetUnarmedDamageDie(NWNXLib::API::CNWSCreatureStats*);
    static int32_t CNWSMessage__SendServerToPlayerParty_List(NWNXLib::API::CNWSMessage*, uint32_t, int32_t, uint32_t*, uint8_t, uint32_t);
    static int32_t CNWSMessage__WriteGameObjUpdate_PartyAIState(NWNXLib::API::CNWSMessage*, NWNXLib::API::CNWSPlayer*);
    static NWNXLib::API::CWorldTimer* CServerExoAppInternal__GetActiveTimer(NWNXLib::API::CServerExoAppInternal*, uint32_t);
    static bool IsOnExclusionList(uint32_t, NWNXLib::API::CExoLinkedListInternal*);
    static void InitTimer(uint32_t);
    static void ResetTimer(uint32_t);
};

}