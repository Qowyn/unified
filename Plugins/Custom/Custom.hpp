#pragma once

#include "Plugin.hpp"
#include "Services/Events/Events.hpp"
#include "API/CNWSCombatRound.hpp"
#include "API/CNWSCreatureStats.hpp"
#include "API/CNWSPlayer.hpp"
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
    NWNXLib::API::CNWSPlayer *player(ArgumentStack& args);
    static void InitializeNumberOfAttacksHook(NWNXLib::API::CNWSCombatRound*);
    static int GetLevelByClass(NWNXLib::API::CNWSCreatureStats *pStats, uint32_t nClassType);
};

}