#include "Custom.hpp"
#include "API/CAppManager.hpp"
#include "API/CExoLinkedListInternal.hpp"
#include "API/CExoLinkedListNode.hpp"
#include "API/CFactionManager.hpp"
#include "API/CGameObjectArray.hpp"
#include "API/CGameObjectArrayNode.hpp"
#include "API/CServerExoApp.hpp"
#include "API/CServerExoAppInternal.hpp"
#include "API/CNWSCombatRound.hpp"
#include "API/CNWSCreature.hpp"
#include "API/CNWSCreatureStats.hpp"
#include "API/CNWSFaction.hpp"
#include "API/CNWSModule.hpp"
#include "API/CNWSPlayer.hpp"
#include "API/CWorldTimer.hpp"
#include "API/Functions.hpp"
#include "API/Globals.hpp"
#include "API/Constants.hpp"
#include "API/Types.hpp"
#include "Services/Events/Events.hpp"
#include "Services/Hooks/Hooks.hpp"
#include "Services/Messaging/Messaging.hpp"
#include "Services/PerObjectStorage/PerObjectStorage.hpp"
#include "Utils.hpp"

using namespace NWNXLib;
using namespace NWNXLib::API;
using namespace NWNXLib::API::Constants;

static Custom::Custom* g_plugin;

NWNX_PLUGIN_ENTRY Plugin::Info* PluginInfo()
{
    return new Plugin::Info
    {
        "Custom",
        "Misc changes",
        "Qowyn",
        "qowync@gmail.com",
        1,
        true
    };
}

NWNX_PLUGIN_ENTRY Plugin* PluginLoad(Plugin::CreateParams params)
{
    g_plugin = new Custom::Custom(params);
    return g_plugin;
}

namespace Custom {

static NWNXLib::Hooking::FunctionHook* m_InitializeNumberOfAttacksHook;
static NWNXLib::Hooking::FunctionHook* m_GetFavoredEnemyBonusHook;
static NWNXLib::Hooking::FunctionHook* m_SetAreaHook;
static NWNXLib::Hooking::FunctionHook* m_HandlePlayerToServerInputMessageHook;
static NWNXLib::Hooking::FunctionHook* m_CNWSMessage__SendServerToPlayerParty_ListHook;
static NWNXLib::Hooking::FunctionHook* m_CNWSMessage__WriteGameObjUpdate_PartyAIStateHook;
static NWNXLib::Hooking::FunctionHook* m_CWorldTimer__PauseWorldTimerHook;
static NWNXLib::Hooking::FunctionHook* m_CNWSMessage__SendServerToPlayerChatMultiLangMessageHook;
static NWNXLib::Hooking::FunctionHook* m_CNWSMessage__SendServerToPlayerChatMultiLang_HelperHook;
static std::unordered_map<Types::ObjectID, std::vector<Types::ObjectID>> timeStopAreas;
static std::vector<Types::ObjectID> pauseAreas;
static std::unordered_map<Types::ObjectID, CWorldTimer> areaTimers;

constexpr uintptr_t ChatMultiLang_HelperInlined = 0x00000000002c7440;

Custom::Custom(const Plugin::CreateParams& params)
    : Plugin(params)
{
#define REGISTER(func) \
    GetServices()->m_events->RegisterEvent(#func, std::bind(&Custom::func, this, std::placeholders::_1))

    REGISTER(PossessCreature);
    REGISTER(UnpossessCreature);
    REGISTER(GetFactionCount);
    REGISTER(GetFactionId);
    REGISTER(SetFactionId);
    REGISTER(GetFactionName);
    REGISTER(GetSuppressDialog);
    REGISTER(SetSuppressDialog);
    REGISTER(SetHasInventory);
#undef REGISTER
    GetServices()->m_hooks->RequestExclusiveHook<Functions::_ZN15CNWSCombatRound25InitializeNumberOfAttacksEv>(&InitializeNumberOfAttacks);
    m_InitializeNumberOfAttacksHook = GetServices()->m_hooks->FindHookByAddress(Functions::_ZN15CNWSCombatRound25InitializeNumberOfAttacksEv);
    GetServices()->m_hooks->RequestExclusiveHook<Functions::_ZN17CNWSCreatureStats20GetFavoredEnemyBonusEP12CNWSCreature>(&GetFavoredEnemyBonus);
    m_GetFavoredEnemyBonusHook = GetServices()->m_hooks->FindHookByAddress(Functions::_ZN17CNWSCreatureStats20GetFavoredEnemyBonusEP12CNWSCreature);
    /*
    GetServices()->m_hooks->RequestExclusiveHook<Functions::_ZN21CNWSEffectListHandler15OnApplyTimestopEP10CNWSObjectP11CGameEffecti>(&CNWSEffectListHandler__OnApplyTimestop);
    GetServices()->m_hooks->RequestExclusiveHook<Functions::_ZN21CNWSEffectListHandler16OnRemoveTimestopEP10CNWSObjectP11CGameEffect>(&CNWSEffectListHandler__OnRemoveTimestop);
    GetServices()->m_hooks->RequestExclusiveHook<Functions::_ZN10CNWSObject7SetAreaEP8CNWSArea>(&CNWSObject__SetArea);
    m_SetAreaHook = GetServices()->m_hooks->FindHookByAddress(Functions::_ZN10CNWSObject7SetAreaEP8CNWSArea);
    GetServices()->m_hooks->RequestExclusiveHook<Functions::_ZN11CNWSMessage38SendServerToPlayerModule_SetPauseStateEhi>(&CNWSMessage__SendServerToPlayerModule_SetPauseState);
    GetServices()->m_hooks->RequestExclusiveHook<Functions::_ZN21CServerExoAppInternal23IsOnActiveExclusionListEj>(&CServerExoAppInternal__IsOnActiveExclusionList);
    GetServices()->m_hooks->RequestExclusiveHook<Functions::_ZN21CServerExoAppInternal17IsOnExclusionListEj>(&CServerExoAppInternal__IsOnActiveExclusionList);
    GetServices()->m_hooks->RequestExclusiveHook<Functions::_ZN11CNWSMessage32HandlePlayerToServerInputMessageEP10CNWSPlayerh>(&CNWSMessage__HandlePlayerToServerInputMessage);
    m_HandlePlayerToServerInputMessageHook = GetServices()->m_hooks->FindHookByAddress(Functions::_ZN11CNWSMessage32HandlePlayerToServerInputMessageEP10CNWSPlayerh);
    */
    GetServices()->m_hooks->RequestExclusiveHook<Functions::_ZN17CNWSCreatureStats20GetUnarmedDamageDiceEv>(&CNWSCreatureStats__GetUnarmedDamageDice);
    GetServices()->m_hooks->RequestExclusiveHook<Functions::_ZN17CNWSCreatureStats19GetUnarmedDamageDieEv>(&CNWSCreatureStats__GetUnarmedDamageDie);
    GetServices()->m_hooks->RequestExclusiveHook<Functions::_ZN11CNWSMessage28SendServerToPlayerParty_ListEjiPjhj>(&CNWSMessage__SendServerToPlayerParty_List);
    m_CNWSMessage__SendServerToPlayerParty_ListHook = GetServices()->m_hooks->FindHookByAddress(Functions::_ZN11CNWSMessage28SendServerToPlayerParty_ListEjiPjhj);
    GetServices()->m_hooks->RequestExclusiveHook<Functions::_ZN11CNWSMessage31WriteGameObjUpdate_PartyAIStateEP10CNWSPlayer>(&CNWSMessage__WriteGameObjUpdate_PartyAIState);
    m_CNWSMessage__WriteGameObjUpdate_PartyAIStateHook = GetServices()->m_hooks->FindHookByAddress(Functions::_ZN11CNWSMessage31WriteGameObjUpdate_PartyAIStateEP10CNWSPlayer);
    /*
    GetServices()->m_hooks->RequestExclusiveHook<Functions::_ZN11CWorldTimer15PauseWorldTimerEv>(&CWorldTimer__PauseWorldTimer);
    m_CWorldTimer__PauseWorldTimerHook = GetServices()->m_hooks->FindHookByAddress(Functions::_ZN11CWorldTimer15PauseWorldTimerEv);
    GetServices()->m_hooks->RequestExclusiveHook<Functions::_ZN11CWorldTimer17UnpauseWorldTimerEv>(&CWorldTimer__UnpauseWorldTimer);
    GetServices()->m_hooks->RequestSharedHook<API::Functions::_ZN13CServerExoApp8MainLoopEv, int32_t>(
            +[](Services::Hooks::CallType type, CServerExoAppInternal* pInternal)
            {
                if (type == Services::Hooks::CallType::BEFORE_ORIGINAL)
                {
                    if (pInternal->m_nPauseState != 0)
                    {
                        pInternal->m_pWorldTimer->SetSnapshotTime();
                    }
                }
            });
    GetServices()->m_hooks->RequestExclusiveHook<Functions::_ZN13CServerExoApp19GetActivePauseStateEv>(&CServerExoApp__GetActivePauseState);
    GetServices()->m_hooks->RequestExclusiveHook<Functions::_ZN21CServerExoAppInternal14GetActiveTimerEj>(&CServerExoAppInternal__GetActiveTimer);
    */

    GetServices()->m_hooks->RequestExclusiveHook<Functions::_ZN11CNWSMessage38SendServerToPlayerChatMultiLangMessageEhj13CExoLocStringjhPjjiRK7CResRefij>(&SendServerToPlayerChatMultiLangMessage);
    m_CNWSMessage__SendServerToPlayerChatMultiLangMessageHook = GetServices()->m_hooks->FindHookByAddress(Functions::_ZN11CNWSMessage38SendServerToPlayerChatMultiLangMessageEhj13CExoLocStringjhPjjiRK7CResRefij);
    GetServices()->m_hooks->RequestExclusiveHook<ChatMultiLang_HelperInlined>(&SendServerToPlayerChatMultiLang_Helper);
    m_CNWSMessage__SendServerToPlayerChatMultiLang_HelperHook = GetServices()->m_hooks->FindHookByAddress(ChatMultiLang_HelperInlined);
}

Custom::~Custom()
{
}

ArgumentStack Custom::PossessCreature(ArgumentStack&& args)
{
    ArgumentStack stack;
    if (auto *pPlayer = player(args))
    {
        const auto creatureId = Services::Events::ExtractArgument<Types::ObjectID>(args);

        auto *pOldCreature = Globals::AppManager()->m_pServerExoApp->GetCreatureByGameObjectID(pPlayer->m_oidNWSObject);
        auto *pNewCreature = Globals::AppManager()->m_pServerExoApp->GetCreatureByGameObjectID(creatureId);
        if (pNewCreature && pOldCreature && pNewCreature->m_nAssociateType == 0 && !pNewCreature->m_pStats->m_bIsPC && !pNewCreature->m_pStats->m_bIsDM && !pNewCreature->GetDead())
        {
            auto *pArea = Globals::AppManager()->m_pServerExoApp->GetAreaByGameObjectID(pNewCreature->m_oidArea);
            pOldCreature->PossessCreature(creatureId);
            if (!pOldCreature->m_pStats->m_bIsPC)
            {
                pOldCreature->m_oidMaster = 0;
                pOldCreature->m_nAssociateType = 0;
            }
            pNewCreature->m_oidMaster = pPlayer->m_oidPCObject;
            pNewCreature->m_nAssociateType = 7;
            if (pOldCreature->m_oidArea != pNewCreature->m_oidArea && pArea)
            {
                pNewCreature->m_oidDesiredArea = pNewCreature->m_oidArea;
                pNewCreature->m_vDesiredAreaLocation = pNewCreature->m_vPosition;
                pNewCreature->m_bDesiredAreaUpdateComplete = 0;

                auto *pMessage = static_cast<CNWSMessage*>(Globals::AppManager()->m_pServerExoApp->GetNWSMessage());
                pMessage->SendServerToPlayerArea_ClientArea(pPlayer, pArea, pNewCreature->m_vPosition.x, pNewCreature->m_vPosition.y, pNewCreature->m_vPosition.z, pNewCreature->m_vOrientation, 0);
            }
        }
    }

    return stack;
}

ArgumentStack Custom::UnpossessCreature(ArgumentStack&& args)
{
    ArgumentStack stack;
    if (auto *pPlayer = player(args))
    {
        auto *pOldCreature = Globals::AppManager()->m_pServerExoApp->GetCreatureByGameObjectID(pPlayer->m_oidNWSObject);
        auto *pNewCreature = Globals::AppManager()->m_pServerExoApp->GetCreatureByGameObjectID(pPlayer->m_oidPCObject);
        if (pNewCreature && pOldCreature && pOldCreature != pNewCreature)
        {
            auto *pArea = Globals::AppManager()->m_pServerExoApp->GetAreaByGameObjectID(pNewCreature->m_oidArea);
            pOldCreature->PossessCreature(pPlayer->m_oidPCObject);
            pOldCreature->m_oidMaster = 0;
            pOldCreature->m_nAssociateType = 0;
            if (pOldCreature->m_oidArea != pNewCreature->m_oidArea && pArea)
            {
                pNewCreature->m_oidDesiredArea = pNewCreature->m_oidArea;
                pNewCreature->m_vDesiredAreaLocation = pNewCreature->m_vPosition;
                pNewCreature->m_bDesiredAreaUpdateComplete = 0;

                auto *pMessage = static_cast<CNWSMessage*>(Globals::AppManager()->m_pServerExoApp->GetNWSMessage());
                pMessage->SendServerToPlayerArea_ClientArea(pPlayer, pArea, pNewCreature->m_vPosition.x, pNewCreature->m_vPosition.y, pNewCreature->m_vPosition.z, pNewCreature->m_vOrientation, 0);
            }
        }
    }

    return stack;
}

ArgumentStack Custom::GetFactionCount(ArgumentStack&&)
{
    ArgumentStack stack;
    int32_t retVal = Globals::AppManager()->m_pServerExoApp->m_pcExoAppInternal->m_pFactionManager->m_nNPCFactionCount;

    Services::Events::InsertArgument(stack, retVal);

    return stack;
}

ArgumentStack Custom::GetFactionId(ArgumentStack&& args)
{
    ArgumentStack stack;
    int32_t retVal = -1;

    const auto nObjectId = Services::Events::ExtractArgument<Types::ObjectID>(args);
    if (nObjectId != OBJECT_INVALID)
    {
        Globals::AppManager()->m_pServerExoApp->GetFactionOfObject(nObjectId, &retVal);
    }
    Services::Events::InsertArgument(stack, retVal);

    return stack;
}

ArgumentStack Custom::SetFactionId(ArgumentStack&& args)
{
    ArgumentStack stack;
    
    const auto nObjectId = Services::Events::ExtractArgument<Types::ObjectID>(args);
    const auto nFactionId = Services::Events::ExtractArgument<int32_t>(args);

    auto *pFaction = Globals::AppManager()->m_pServerExoApp->m_pcExoAppInternal->m_pFactionManager->GetFaction(nFactionId);
    if (pFaction)
    {
        pFaction->AddMember(nObjectId, 0);
    }

    return stack;
}

ArgumentStack Custom::GetFactionName(ArgumentStack&& args)
{
    ArgumentStack stack;
    std::string retVal;

    auto nFactionId = Services::Events::ExtractArgument<int32_t>(args);

    auto *pFaction = Globals::AppManager()->m_pServerExoApp->m_pcExoAppInternal->m_pFactionManager->GetFaction(nFactionId);
    if (pFaction && pFaction->m_sFactionName.CStr())
    {
        retVal = std::string(pFaction->m_sFactionName.CStr());
    }

    Services::Events::InsertArgument(stack, retVal);

    return stack;
}

ArgumentStack Custom::GetSuppressDialog(ArgumentStack&& args)
{
    ArgumentStack stack;

    int32_t retVal = 0;
    if (auto *pPlayer = player(args))
    {
        const auto nObjectId = Services::Events::ExtractArgument<Types::ObjectID>(args);
        if (auto suppressData = g_plugin->GetServices()->m_perObjectStorage->Get<int>(pPlayer->m_oidPCObject, "SUPPRESS_DIALOG_LOG!" + Utils::ObjectIDToString(nObjectId)))
        {
            retVal = *suppressData;
        }
    }
    Services::Events::InsertArgument(stack, retVal);

    return stack;
}

ArgumentStack Custom::SetSuppressDialog(ArgumentStack&& args)
{
    ArgumentStack stack;

    if (auto *pPlayer = player(args))
    {
        const auto nObjectId = Services::Events::ExtractArgument<Types::ObjectID>(args);
        const auto bValue = Services::Events::ExtractArgument<int32_t>(args);

        if (nObjectId != OBJECT_INVALID)
        {
            g_plugin->GetServices()->m_perObjectStorage->Set(pPlayer->m_oidPCObject, "SUPPRESS_DIALOG_LOG!" + Utils::ObjectIDToString(nObjectId), bValue);
        }
    }

    return stack;
}

ArgumentStack Custom::SetHasInventory(ArgumentStack&& args)
{
    ArgumentStack stack;

    const auto nObjectId = Services::Events::ExtractArgument<Types::ObjectID>(args);
    const auto bValue = Services::Events::ExtractArgument<int32_t>(args);

    if (auto *pPLC = Globals::AppManager()->m_pServerExoApp->GetPlaceableByGameObjectID(nObjectId))
    {
        pPLC->m_bHasInventory = bValue;
    }

    return stack;
}

CNWSPlayer *Custom::player(ArgumentStack& args)
{
    const auto playerId = Services::Events::ExtractArgument<Types::ObjectID>(args);

    if (playerId == Constants::OBJECT_INVALID)
    {
        LOG_NOTICE("NWNX_Custom function called on OBJECT_INVALID");
        return nullptr;
    }

    auto *pPlayer = Globals::AppManager()->m_pServerExoApp->GetClientObjectByObjectId(playerId);
    if (!pPlayer)
    {
        LOG_NOTICE("NWNX_Custom function called on non-player object %x", playerId);
    }
    return pPlayer;
}

void Custom::InitializeNumberOfAttacks(CNWSCombatRound* pRound)
{
    m_InitializeNumberOfAttacksHook->CallOriginal<void>(pRound);
    auto *pStats = pRound->m_pBaseCreature->m_pStats;

    if (pRound->m_pBaseCreature->m_nCombatMode == Constants::CombatMode::FlurryOfBlows)
    {
        int nMonk = GetLevelByClass(pStats, Constants::ClassType::Monk);
        if (nMonk > 10)
        {
            pRound->m_nAdditionalAttacks += 1;
        }
    }
}

int32_t Custom::GetFavoredEnemyBonus(CNWSCreatureStats* pStats, CNWSCreature* pOtherCreature)
{
    return m_GetFavoredEnemyBonusHook->CallOriginal<int32_t>(pStats, pOtherCreature) * 2;
}

int Custom::GetLevelByClass(CNWSCreatureStats *pStats, uint32_t nClassType)
{
    for (int i = 0; i < pStats->m_nNumMultiClasses; i++)
    {
        if (pStats->m_ClassInfo[i].m_nClass == nClassType)
            return pStats->m_ClassInfo[i].m_nLevel;
    }

    return 0;
}

int32_t Custom::CNWSEffectListHandler__OnApplyTimestop(CNWSEffectListHandler*, CNWSObject *pObj, CGameEffect*, int32_t)
{
    if (pObj->m_oidArea != OBJECT_INVALID)
    {
        auto &tsObjects = timeStopAreas[pObj->m_oidArea];
        if (std::find(std::begin(tsObjects), std::end(tsObjects), pObj->m_idSelf) == std::end(tsObjects)) tsObjects.push_back(pObj->m_idSelf);
        InitTimer(pObj->m_oidArea);

        auto *pServer = Globals::AppManager()->m_pServerExoApp;
        auto bWasTS = pServer->GetPauseState(1) != 0;
        auto bIsTS = timeStopAreas.size() > 0;

        pServer->AddToExclusionList(pObj->m_idSelf, 1);

        if (bWasTS != bIsTS)
        {
            pServer->TogglePauseState(1);
        }
        else if (bIsTS)
        {
            auto *pMessage = static_cast<CNWSMessage*>(pServer->GetNWSMessage());
            pMessage->SendServerToPlayerModule_SetPauseState(1, 1);
        }
    }
    return 0;
}

int32_t Custom::CNWSEffectListHandler__OnRemoveTimestop(CNWSEffectListHandler*, CNWSObject *pObj, CGameEffect*)
{
    if (pObj->m_oidArea != OBJECT_INVALID)
    {
        auto bAllTimestopsGone = TimestopCount(pObj) < 2;

        if (!bAllTimestopsGone) return 1;

        auto &tsObjects = timeStopAreas.at(pObj->m_oidArea);
        tsObjects.erase(std::find(std::begin(tsObjects), std::end(tsObjects), pObj->m_idSelf));
        if (!tsObjects.size())
        {
            timeStopAreas.erase(pObj->m_oidArea);
            ResetTimer(pObj->m_oidArea);
        }

        if (pObj->m_nObjectType != ObjectType::Creature || !IsDM(static_cast<CNWSCreature*>(pObj)))
        {
            Globals::AppManager()->m_pServerExoApp->RemoveFromExclusionList(pObj->m_idSelf, 1);
        }

        if (timeStopAreas.size() <= 0)
        {
            Globals::AppManager()->m_pServerExoApp->SetPauseState(1, 0);
        }
        else
        {
            auto *pMessage = static_cast<CNWSMessage*>(Globals::AppManager()->m_pServerExoApp->GetNWSMessage());
            pMessage->SendServerToPlayerModule_SetPauseState(1, 1);
        }
    }
    return 1;
}

void Custom::CNWSObject__SetArea(CNWSObject* pObj, CNWSArea* pArea)
{
    Types::ObjectID oldAreaId = pObj->m_oidArea;
    Types::ObjectID newAreaId = pArea ? pArea->m_idSelf : OBJECT_INVALID;

    m_SetAreaHook->CallOriginal<void>(pObj, pArea);

    if (newAreaId != oldAreaId)
    {
        CNWSObject__SetAreaPost(pObj, oldAreaId, newAreaId);
    }
}

void Custom::CNWSObject__SetAreaPost(CNWSObject* pObj, uint32_t oldAreaId, uint32_t newAreaId)
{
    if (!Globals::AppManager()->m_pServerExoApp->m_pcExoAppInternal->m_nPauseState) return;

    if (HasTimestop(pObj))
    {
        if (oldAreaId != OBJECT_INVALID)
        {
            auto &tsObjects = timeStopAreas.at(oldAreaId);
            tsObjects.erase(std::find(std::begin(tsObjects), std::end(tsObjects), pObj->m_idSelf));
            if (!tsObjects.size())
            {
                timeStopAreas.erase(oldAreaId);
                ResetTimer(oldAreaId);
            }
        }

        if (newAreaId != OBJECT_INVALID)
        {
            auto &tsObjects = timeStopAreas[newAreaId];
            tsObjects.push_back(pObj->m_idSelf);
            InitTimer(newAreaId);
        }

        auto bWasTS = Globals::AppManager()->m_pServerExoApp->GetPauseState(1) != 0;
        auto bIsTS = timeStopAreas.size() > 0;

        if (bWasTS != bIsTS)
        {
            Globals::AppManager()->m_pServerExoApp->TogglePauseState(1);
        }
        else if (bIsTS)
        {
            auto *pMessage = static_cast<CNWSMessage*>(Globals::AppManager()->m_pServerExoApp->GetNWSMessage());
            pMessage->SendServerToPlayerModule_SetPauseState(1, 1);
        }

        if (pObj->m_nObjectType != ObjectType::Creature) return;

        if (auto *pClient = Globals::AppManager()->m_pServerExoApp->GetClientObjectByObjectId(pObj->m_idSelf))
        {
            auto bWasPause = std::find(pauseAreas.begin(), pauseAreas.end(), oldAreaId) != pauseAreas.end();
            auto bIsPause = std::find(pauseAreas.begin(), pauseAreas.end(), newAreaId) != pauseAreas.end();
            if (bIsPause != bWasPause)
            {
                auto *pMessage = static_cast<CNWSMessage*>(Globals::AppManager()->m_pServerExoApp->GetNWSMessage());
                SendPauseState(pMessage, 2, bIsPause, pClient->m_nPlayerID);
            }
        }
        return;
    }

    if (pObj->m_nObjectType == ObjectType::Creature)
    {
        if (auto *pClient = Globals::AppManager()->m_pServerExoApp->GetClientObjectByObjectId(pObj->m_idSelf))
        {
            auto bWasTS = timeStopAreas.find(oldAreaId) != timeStopAreas.end();
            auto bIsTS = timeStopAreas.find(newAreaId) != timeStopAreas.end();
            if (bIsTS != bWasTS)
            {
                auto *pMessage = static_cast<CNWSMessage*>(Globals::AppManager()->m_pServerExoApp->GetNWSMessage());
                SendPauseState(pMessage, 1, bIsTS, pClient->m_nPlayerID);
            }
            auto bWasPause = std::find(pauseAreas.begin(), pauseAreas.end(), oldAreaId) != pauseAreas.end();
            auto bIsPause = std::find(pauseAreas.begin(), pauseAreas.end(), newAreaId) != pauseAreas.end();
            if (bIsPause != bWasPause)
            {
                auto *pMessage = static_cast<CNWSMessage*>(Globals::AppManager()->m_pServerExoApp->GetNWSMessage());
                SendPauseState(pMessage, 2, bIsPause, pClient->m_nPlayerID);
            }
        }
    }
    
}

int32_t Custom::CNWSMessage__SendServerToPlayerModule_SetPauseState(CNWSMessage* pMessage, uint8_t nPauseState, int32_t bPaused)
{
    auto *pServer = Globals::AppManager()->m_pServerExoApp->m_pcExoAppInternal;
    auto *pPlayerList = pServer->m_pNWSPlayerList->m_pcExoLinkedListInternal;
    if (bPaused)
    {
        // Get Player Counts
        uint32_t nSeePause = 0;
        auto nPlayerCount = pPlayerList->m_nCount;
        for (auto *pPlayerNode = pPlayerList->pHead; pPlayerNode; pPlayerNode = pPlayerNode->pNext)
        {
            if (auto *pPlayer = static_cast<CNWSPlayer*>(pPlayerNode->pObject))
            {
                if (auto *pCreature = pServer->GetCreatureByGameObjectID(pPlayer->m_oidNWSObject))
                {
                    if (nPauseState == 1 && timeStopAreas.find(pCreature->m_oidArea) != timeStopAreas.end())
                    {
                        nSeePause++;
                    }
                    else if (nPauseState == 2 && std::find(pauseAreas.begin(), pauseAreas.end(), pCreature->m_oidArea) != pauseAreas.end())
                    {
                        nSeePause++;
                    }
                }
            }
        }

        if (nSeePause > 0)
        {
            unsigned char* szMessage;
            uint32_t messageSize;
            pMessage->CreateWriteMessage(1, 0xFFFFFFFF, 1);
            pMessage->WriteBYTE(nPauseState, 2);
            pMessage->WriteBOOL(1);
            auto *pExclusionList = nPauseState == 1 ? pServer->m_lstTimestopExclusionList : pServer->m_lstPauseExclusionList;
            pMessage->WriteWORD(pExclusionList->num, 16);
            for (int32_t index = 0; index < pExclusionList->num; index++)
            {
                pMessage->WriteOBJECTIDServer(pExclusionList->element[index]);
            }

            pMessage->GetWriteMessage(&szMessage, &messageSize);

            for (auto *pPlayerNode = pPlayerList->pHead; pPlayerNode; pPlayerNode = pPlayerNode->pNext)
            {
                if (auto *pPlayer = static_cast<CNWSPlayer*>(pPlayerNode->pObject))
                {
                    if (auto *pCreature = pServer->GetCreatureByGameObjectID(pPlayer->m_oidNWSObject))
                    {
                        if (nPauseState == 1 && timeStopAreas.find(pCreature->m_oidArea) != timeStopAreas.end())
                        {
                            pMessage->SendServerToPlayerMessage(pPlayer->m_nPlayerID, MessageMajor::Module, MessageModuleMinor::SetPauseState, szMessage, messageSize);
                        }
                        else if (nPauseState == 2 && std::find(pauseAreas.begin(), pauseAreas.end(), pCreature->m_oidArea) != pauseAreas.end())
                        {
                            pMessage->SendServerToPlayerMessage(pPlayer->m_nPlayerID, MessageMajor::Module, MessageModuleMinor::SetPauseState, szMessage, messageSize);
                        }
                    }
                }
            }
        }

        if (nPlayerCount > nSeePause)
        {
            unsigned char* szMessage;
            uint32_t messageSize;
            pMessage->CreateWriteMessage(1, 0xFFFFFFFF, 1);
            pMessage->WriteBYTE(pServer->m_nPauseState, 2);
            pMessage->WriteBOOL(0);
            auto *pExclusionList = nPauseState == 2 ? pServer->m_lstTimestopExclusionList : pServer->m_lstPauseExclusionList;
            pMessage->WriteWORD(pExclusionList->num, 16);
            for (int32_t index = 0; index < pExclusionList->num; index++)
            {
                pMessage->WriteOBJECTIDServer(pExclusionList->element[index]);
            }
            pMessage->GetWriteMessage(&szMessage, &messageSize);

            for (auto *pPlayerNode = pPlayerList->pHead; pPlayerNode; pPlayerNode = pPlayerNode->pNext)
            {
                if (auto *pPlayer = static_cast<CNWSPlayer*>(pPlayerNode->pObject))
                {
                    if (auto *pCreature = pServer->GetCreatureByGameObjectID(pPlayer->m_oidNWSObject))
                    {
                        if (nPauseState == 1 && timeStopAreas.find(pCreature->m_oidArea) == timeStopAreas.end())
                        {
                            pMessage->SendServerToPlayerMessage(pPlayer->m_nPlayerID, MessageMajor::Module, MessageModuleMinor::SetPauseState, szMessage, messageSize);
                        }
                        else if (nPauseState == 2 && std::find(pauseAreas.begin(), pauseAreas.end(), pCreature->m_oidArea) == pauseAreas.end())
                        {
                            pMessage->SendServerToPlayerMessage(pPlayer->m_nPlayerID, MessageMajor::Module, MessageModuleMinor::SetPauseState, szMessage, messageSize);
                        }
                    }
                    else
                    {
                        pMessage->SendServerToPlayerMessage(pPlayer->m_nPlayerID, MessageMajor::Module, MessageModuleMinor::SetPauseState, szMessage, messageSize);
                    }
                }
            }
        }
    }
    else
    {
        unsigned char* szMessage;
        uint32_t messageSize;
        pMessage->CreateWriteMessage(1, 0xFFFFFFFF, 1);
        pMessage->WriteBYTE(nPauseState, 2);
        pMessage->WriteBOOL(0);
        auto *pExclusionList = nPauseState == 2 ? pServer->m_lstTimestopExclusionList : pServer->m_lstPauseExclusionList;
        pMessage->WriteWORD(pExclusionList->num, 16);
        for (int32_t index = 0; index < pExclusionList->num; index++)
        {
            pMessage->WriteOBJECTIDServer(pExclusionList->element[index]);
        }
        pMessage->GetWriteMessage(&szMessage, &messageSize);

        for (auto *pPlayerNode = pPlayerList->pHead; pPlayerNode; pPlayerNode = pPlayerNode->pNext)
        {
            if (auto *pPlayer = static_cast<CNWSPlayer*>(pPlayerNode->pObject))
            {
                pMessage->SendServerToPlayerMessage(pPlayer->m_nPlayerID, MessageMajor::Module, MessageModuleMinor::SetPauseState, szMessage, messageSize);
            }
        }
    }
    return 1;
}

void Custom::SendPauseState(CNWSMessage* pMessage, int32_t nPauseState, bool bPaused, uint32_t nPlayerId)
{
    unsigned char* szMessage;
    uint32_t messageSize;

    auto* pServer = Globals::AppManager()->m_pServerExoApp->m_pcExoAppInternal;
    pMessage->CreateWriteMessage(1, 0xFFFFFFFF, 1);
    pMessage->WriteBYTE(nPauseState, 2);
    pMessage->WriteBOOL(bPaused);

    auto *pExclusionList = (nPauseState == 1) == bPaused ? pServer->m_lstTimestopExclusionList : pServer->m_lstPauseExclusionList;
    pMessage->WriteWORD(pExclusionList->num, 16);
    for (int32_t index = 0; index < pExclusionList->num; index++)
    {
        pMessage->WriteOBJECTIDServer(pExclusionList->element[index]);
    }

    pMessage->GetWriteMessage(&szMessage, &messageSize);

    pMessage->SendServerToPlayerMessage(nPlayerId, MessageMajor::Module, MessageModuleMinor::SetPauseState, szMessage, messageSize);
}

bool Custom::HasTimestop(CNWSObject *pCreature)
{
    auto pEffects = &pCreature->m_appliedEffects;
    if (!pEffects->element) return false;

    for (int32_t n = 0; n < pEffects->num; n++)
    {
        auto *pEffect = pEffects->element[n];
        if (pEffect->m_nType == EffectTrueType::Timestop) return true;
    }

    return false;
}

int32_t Custom::TimestopCount(CNWSObject* pCreature)
{
    auto result = 0;
    auto pEffects = &pCreature->m_appliedEffects;
    if (!pEffects->element) return result;

    for (int32_t n = 0; n < pEffects->num; n++)
    {
        auto *pEffect = pEffects->element[n];
        if (pEffect->m_nType == EffectTrueType::Timestop) result++;
    }

    return result;
}

bool Custom::IsDM(CNWSCreature *pCreature)
{
    if (!pCreature->m_bPlayerCharacter) return false;

    auto *playerList = Globals::AppManager()->m_pServerExoApp->m_pcExoAppInternal->m_pNWSPlayerList->m_pcExoLinkedListInternal;
    for (auto *playerNode = playerList->pHead; playerNode; playerNode = playerNode->pNext)
    {
        auto *player = static_cast<CNWSPlayer*>(playerNode->pObject);
        if (player && player->m_oidPCObject == pCreature->m_idSelf)
        {
            return player->m_nCharacterType == CharacterType::DM;
        }
    }

    return false;
}

int32_t Custom::CServerExoAppInternal__IsOnActiveExclusionList(CServerExoAppInternal* pInternal, uint32_t nObjectId)
{
    if (auto *pGObj = pInternal->GetGameObject(nObjectId))
    {
        if (auto *pObj = Utils::AsNWSObject(pGObj))
        {
            if (std::find(pauseAreas.begin(), pauseAreas.end(), pObj->m_oidArea) != pauseAreas.end() && !IsOnExclusionList(nObjectId, pInternal->m_lstPauseExclusionList))
            {
                return 0;
            }
            if (timeStopAreas.find(pObj->m_oidArea) != timeStopAreas.end() && !IsOnExclusionList(nObjectId, pInternal->m_lstTimestopExclusionList))
            {
                return 0;
            }
        }
    }
    return 1;
}

int32_t Custom::CNWSMessage__HandlePlayerToServerInputMessage(CNWSMessage* pMessage, CNWSPlayer* pPlayer, uint8_t nMinor)
{
    if (nMinor != MessageInputMinor::TogglePauseRequest)
    {
        return m_HandlePlayerToServerInputMessageHook->CallOriginal<int32_t>(pMessage, pPlayer, nMinor);
    }

    if (pPlayer->m_nCharacterType != CharacterType::DM) return 0;

    auto *pPC = pPlayer->GetGameObject();

    if (!pPC || pPC->m_oidArea == OBJECT_INVALID) return 0;

    auto iter = std::find(pauseAreas.begin(), pauseAreas.end(), pPC->m_oidArea);

    if (iter == pauseAreas.end())
    {
        pauseAreas.push_back(pPC->m_oidArea);
        InitTimer(pPC->m_oidArea);
    }
    else
    {
        pauseAreas.erase(iter);
        ResetTimer(pPC->m_oidArea);
    }

    auto bWasPaused = Globals::AppManager()->m_pServerExoApp->GetPauseState(2) != 0;
    auto bIsPaused = pauseAreas.size() > 0;

    if (bWasPaused != bIsPaused)
    {
        Globals::AppManager()->m_pServerExoApp->TogglePauseState(2);
    }
    else if (bIsPaused)
    {
        auto *pMessage = static_cast<CNWSMessage*>(Globals::AppManager()->m_pServerExoApp->GetNWSMessage());
        pMessage->SendServerToPlayerModule_SetPauseState(2, 1);
    }

    return 0;
}

void Custom::CWorldTimer__PauseWorldTimer(CWorldTimer*) {}
void Custom::CWorldTimer__UnpauseWorldTimer(CWorldTimer*) {}

uint8_t Custom::CServerExoApp__GetActivePauseState(CServerExoApp*)
{
    return 0;
}

uint8_t Custom::CNWSCreatureStats__GetUnarmedDamageDice(CNWSCreatureStats* pStats)
{
    if (!pStats->m_pBaseCreature->GetUseMonkAbilities())
    {
        return 1;
    }

    auto nMonkLevel = GetLevelByClass(pStats, ClassType::Monk);
    auto nSize = pStats->m_pBaseCreature->m_nCreatureSize;

    if (nSize < 3)
    {
        return nMonkLevel >= 16 ? 2 : 1;
    }
    else
    {
        return nMonkLevel >= 12 ? 2 : 1;
    }
}

uint8_t Custom::CNWSCreatureStats__GetUnarmedDamageDie(CNWSCreatureStats* pStats)
{
    auto nSize = pStats->m_pBaseCreature->m_nCreatureSize;
    if (!pStats->m_pBaseCreature->GetUseMonkAbilities())
    {
        return nSize < 3 ? 2 : 3;
    }

    auto nMonkLevel = GetLevelByClass(pStats, ClassType::Monk);

    if (nSize < 3)
    {
        if (nMonkLevel >= 20) return 8;
        if (nMonkLevel >= 16) return 6;
        if (nMonkLevel >= 12) return 10;
        if (nMonkLevel >= 8) return 8;
        if (nMonkLevel >= 4) return 6;
        if (nMonkLevel >= 1) return 4;
        return 2;
    }
    else
    {
        if (nMonkLevel >= 20) return 10;
        if (nMonkLevel >= 16) return 8;
        if (nMonkLevel >= 12) return 6;
        if (nMonkLevel >= 8) return 10;
        if (nMonkLevel >= 4) return 8;
        if (nMonkLevel >= 1) return 6;
        return 3;
    }
}

int32_t Custom::CNWSMessage__SendServerToPlayerParty_List(CNWSMessage* pMessage, uint32_t nPlayerID, int32_t nMemberCount, uint32_t* pMembers, uint8_t unk1, uint32_t unk2)
{
    auto *pServer = Globals::AppManager()->m_pServerExoApp;
    if (auto *pClient = static_cast<CNWSPlayer*>(pServer->GetClientObjectByPlayerId(nPlayerID, 0)))
    {
        if (pClient->m_nCharacterType != CharacterType::DM && pClient->m_oidNWSObject != pClient->m_oidPCObject)
        {
            auto *pCreature = pServer->GetCreatureByGameObjectID(pClient->m_oidNWSObject);
            auto *pPC = pServer->GetCreatureByGameObjectID(pClient->m_oidPCObject);
            if (pCreature && pPC && pCreature->m_nAssociateType == 7 && pCreature->m_pStats->m_nFactionId != pPC->m_pStats->m_nFactionId && pServer->m_pcExoAppInternal->m_pFactionManager->GetIsNPCFaction(pCreature->m_pStats->m_nFactionId))
            {
                if (auto *pFaction = pPC->GetFaction())
                {
                    return m_CNWSMessage__SendServerToPlayerParty_ListHook->CallOriginal<int32_t>(pMessage, nPlayerID, pFaction->m_listFactionMembers.num, pFaction->m_listFactionMembers.element, unk1, unk2);
                }
            }
        }
    }
    return m_CNWSMessage__SendServerToPlayerParty_ListHook->CallOriginal<int32_t>(pMessage, nPlayerID, nMemberCount, pMembers, unk1, unk2);
}

int32_t Custom::CNWSMessage__WriteGameObjUpdate_PartyAIState(CNWSMessage* pMessage, CNWSPlayer* pClient)
{
    if (pClient->m_nCharacterType != CharacterType::DM && pClient->m_oidNWSObject != pClient->m_oidPCObject)
    {
        auto *pServer = Globals::AppManager()->m_pServerExoApp;
        auto *pCreature = pServer->GetCreatureByGameObjectID(pClient->m_oidNWSObject);
        auto *pPC = pServer->GetCreatureByGameObjectID(pClient->m_oidPCObject);
        if (pCreature && pPC && pCreature->m_nAssociateType == 7 && pCreature->m_pStats->m_nFactionId != pPC->m_pStats->m_nFactionId && pServer->m_pcExoAppInternal->m_pFactionManager->GetIsNPCFaction(pCreature->m_pStats->m_nFactionId))
        {
            auto oldFactionId = pCreature->m_pStats->m_nFactionId;
            pCreature->m_pStats->m_nFactionId = pPC->m_pStats->m_nFactionId;
            auto result = m_CNWSMessage__WriteGameObjUpdate_PartyAIStateHook->CallOriginal<int32_t>(pMessage, pClient);
            pCreature->m_pStats->m_nFactionId = oldFactionId;
            return result;
        }
    }
    return m_CNWSMessage__WriteGameObjUpdate_PartyAIStateHook->CallOriginal<int32_t>(pMessage, pClient);
}

CWorldTimer* Custom::CServerExoAppInternal__GetActiveTimer(CServerExoAppInternal* pInternal, uint32_t nOID)
{
    if (pInternal->m_nPauseState == 0) return pInternal->m_pWorldTimer;
    if (auto *pObj = Utils::AsNWSObject(pInternal->GetGameObject(nOID)))
    {
        if (std::find(pauseAreas.begin(), pauseAreas.end(), pObj->m_oidArea) != pauseAreas.end() && !IsOnExclusionList(nOID, pInternal->m_lstPauseExclusionList))
        {
            return &areaTimers.at(pObj->m_oidArea);
        }
        if (timeStopAreas.find(pObj->m_oidArea) != timeStopAreas.end() && !IsOnExclusionList(nOID, pInternal->m_lstTimestopExclusionList))
        {
            return &areaTimers.at(pObj->m_oidArea);
        }
    }
    return pInternal->m_pWorldTimer;
}

bool Custom::IsOnExclusionList(uint32_t nOID, AurList<OBJECT_ID>* pExclusionList)
{
    for (int32_t index = 0; index < pExclusionList->num; index++)
    {
        if (pExclusionList->element[index] == nOID) return true;
    }
    return false;
}

void Custom::InitTimer(uint32_t nArea)
{
    auto &areaTimer = areaTimers[nArea];
    if (!areaTimer.m_bPaused)
    {
        auto *worldTimer = Globals::AppManager()->m_pServerExoApp->m_pcExoAppInternal->m_pWorldTimer;
        areaTimer.m_nTimeSnapshot = worldTimer->m_nTimeSnapshot;
        areaTimer.m_nMillisecondsPerDay = worldTimer->m_nMillisecondsPerDay;
        areaTimer.m_nCalendarDayAdjustment = worldTimer->m_nCalendarDayAdjustment;
        areaTimer.m_nTimeOfDayAdjustment = worldTimer->m_nTimeOfDayAdjustment;
        m_CWorldTimer__PauseWorldTimerHook->CallOriginal<void>(areaTimer);
    }
}

void Custom::ResetTimer(uint32_t nArea)
{
    if (std::find(pauseAreas.begin(), pauseAreas.end(), nArea) == pauseAreas.end() && timeStopAreas.find(nArea) == timeStopAreas.end())
    {
        areaTimers[nArea].m_bPaused = false;
    }
}

int32_t Custom::SendServerToPlayerChatMultiLangMessage(CNWSMessage *pMessage, uint8_t nChatMessageType, OBJECT_ID oidSpeaker, CExoLocString sSpeakerMessage, OBJECT_ID oidTokenTarget, uint8_t gender, uint32_t * pPlayerIdNoBubble, uint32_t nPlayerIdNoBubble, int32_t bPrivateChat, const CResRef * sSound, int32_t bPlayHelloSound, OBJECT_ID oidLastSpeaker)
{
    auto bSuppress = g_plugin->GetServices()->m_perObjectStorage->Get<int>(oidSpeaker, "SUPPRESS_DIALOG_LOG");

    if (bSuppress && *bSuppress)
    {
        return 0;
    }

    return m_CNWSMessage__SendServerToPlayerChatMultiLangMessageHook->CallOriginal<int32_t>(pMessage, nChatMessageType, oidSpeaker, sSpeakerMessage, oidTokenTarget, gender, pPlayerIdNoBubble, nPlayerIdNoBubble, bPrivateChat, sSound, bPlayHelloSound, oidLastSpeaker);
}

int32_t Custom::SendServerToPlayerChatMultiLang_Helper(CNWSMessage *pMessage, uint32_t nPlayerID, OBJECT_ID oidSpeaker, CExoLocString sSpeakerMessage, OBJECT_ID oidTokenTarget, uint8_t gender, BOOL bNoBubble, const CResRef * sSound, BOOL bPlayHelloSound, OBJECT_ID oidLastSpeaker)
{
    CNWSClient* client = Globals::AppManager()->m_pServerExoApp->GetClientObjectByPlayerId(nPlayerID, 0);

    if (!client)
    {
        return 0;
    }

    auto bSuppress = g_plugin->GetServices()->m_perObjectStorage->Get<int>(static_cast<CNWSPlayer*>(client)->m_oidPCObject, "SUPPRESS_DIALOG_LOG!" + Utils::ObjectIDToString(oidSpeaker));

    if (bSuppress && *bSuppress)
    {
        return 0;
    }

    return m_CNWSMessage__SendServerToPlayerChatMultiLang_HelperHook->CallOriginal<int32_t>(pMessage, nPlayerID, oidSpeaker, sSpeakerMessage, oidTokenTarget, gender, bNoBubble, sSound, bPlayHelloSound, oidLastSpeaker);
}

}