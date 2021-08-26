#include "UI/ScribbleSettingsViewController.hpp"

#include "questui/shared/BeatSaberUI.hpp"
#include "config.hpp"

DEFINE_TYPE(Scribble, ScribbleSettingsViewController);

using namespace QuestUI;
namespace Scribble
{
    void ScribbleSettingsViewController::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling)
    {
        if (firstActivation)
        {
            auto container = BeatSaberUI::CreateScrollableSettingsContainer(get_transform());

            auto toggle = BeatSaberUI::CreateToggle(container->get_transform(), "Visible during play", config.visibleDuringPlay, [](bool val){ config.visibleDuringPlay = val; });
            BeatSaberUI::AddHoverHint(toggle->get_gameObject(), "Whether or not your drawing is visible during gameplay");
        }
    }
}