#include "hooks.hpp"
#include "logging.hpp"

#include "custom-types/shared/register.hpp"
#include "questui/shared/QuestUI.hpp"

using namespace Scribble;

extern "C" void setup(ModInfo& info)
{
    info.id = ID;
    info.version = VERSION;
}

extern "C" void load()
{
    QuestUI::Init();

    Hooks::InstallHooks(::Scribble::Logging::getLogger());
    custom_types::Register::AutoRegister();
}