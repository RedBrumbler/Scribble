#include "hooks.hpp"
#include "logging.hpp"

#include "custom-types/shared/register.hpp"
#include "questui/shared/QuestUI.hpp"

#include "VRUIControls/VRPointer.hpp"
#include "GlobalNamespace/VRController.hpp"

#include "CustomTypes/BrushBehaviour.hpp"

using namespace Scribble;

MAKE_AUTO_HOOK_MATCH(VRPointer_OnEnable, &VRUIControls::VRPointer::OnEnable, void, VRUIControls::VRPointer* self)
{
    auto leftController = self->leftVRController;
    auto rightController = self->rightVRController;
    if (!leftController->get_gameObject()->GetComponent<BrushBehaviour*>())
    {
        auto brush = leftController->get_gameObject()->AddComponent<BrushBehaviour*>();
        brush->saberType = GlobalNamespace::SaberType::SaberA;
        brush->pointer = self;
    }   

    if (!rightController->get_gameObject()->GetComponent<BrushBehaviour*>())
    {
        auto brush = rightController->get_gameObject()->AddComponent<BrushBehaviour*>();
        brush->saberType = GlobalNamespace::SaberType::SaberB;
        brush->pointer = self;
    }
}

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