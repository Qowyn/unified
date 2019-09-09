#pragma once

#include "Plugin.hpp"
#include "API/CNWSCombatRound.hpp"
#include "API/CNWSCreatureStats.hpp"
#include "API/Types.hpp"

namespace Custom {

class Custom : public NWNXLib::Plugin
{
public:
    Custom(const Plugin::CreateParams& params);
    virtual ~Custom();

private:
    static void InitializeNumberOfAttacksHook(NWNXLib::API::CNWSCombatRound*);
    static int GetLevelByClass(NWNXLib::API::CNWSCreatureStats *pStats, uint32_t nClassType);
};

}