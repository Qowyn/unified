#include "Custom.hpp"
#include "API/CNWSCombatRound.hpp"
#include "API/CNWSCreature.hpp"
#include "API/CNWSCreatureStats.hpp"
#include "API/Functions.hpp"
#include "API/Globals.hpp"
#include "API/Constants.hpp"
#include "API/Types.hpp"
#include "Services/Hooks/Hooks.hpp"
#include "Services/Messaging/Messaging.hpp"
#include "ViewPtr.hpp"
#include "Utils.hpp"

using namespace NWNXLib;
using namespace NWNXLib::API;
using namespace NWNXLib::API::Constants;

static ViewPtr<Custom::Custom> g_plugin;

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

Custom::Custom(const Plugin::CreateParams& params)
    : Plugin(params)
{
    GetServices()->m_hooks->RequestExclusiveHook<API::Functions::CNWSCombatRound__InitializeNumberOfAttacks>(&InitializeNumberOfAttacksHook);
    m_InitializeNumberOfAttacksHook = GetServices()->m_hooks->FindHookByAddress(Functions::CNWSCombatRound__InitializeNumberOfAttacks);
}

Custom::~Custom()
{
}

void Custom::InitializeNumberOfAttacksHook(API::CNWSCombatRound* pRound)
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
    /*
    if (pStats->HasFeat(1116)) // Combat Reflexes
    {
        int nDex = pStats->GetDEXMod(0);
        if (nDex > 0)
        {
            pRound->m_nAttacksOfOpportunity += nDex;
        }
    }
    */
}

int Custom::GetLevelByClass(NWNXLib::API::CNWSCreatureStats *pStats, uint32_t nClassType)
{
    for (int i = 0; i < pStats->m_nNumMultiClasses; i++)
    {
        if (pStats->m_ClassInfo[i].m_nClass == nClassType)
            return pStats->m_ClassInfo[i].m_nLevel;
    }

    return 0;
}

}