#include "UI/ScribbleSettingsViewController.hpp"

#include "ScribbleContainer.hpp"
#include "questui/shared/BeatSaberUI.hpp"
#include "config.hpp"

#include "GlobalNamespace/MainEffectContainerSO.hpp"
#include "GlobalNamespace/BoolSO.hpp"
#include "UnityEngine/Resources.hpp"

DEFINE_TYPE(Scribble, ScribbleSettingsViewController);

using namespace QuestUI;
using namespace UnityEngine;
namespace Scribble
{
    void ScribbleSettingsViewController::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling)
    {
        if (firstActivation)
        {
            auto container = BeatSaberUI::CreateScrollableSettingsContainer(get_transform());

            auto toggle = BeatSaberUI::CreateToggle(container->get_transform(), "Visible during play", config.visibleDuringPlay, [](bool val){ config.visibleDuringPlay = val; SaveConfig(); });
            BeatSaberUI::AddHoverHint(toggle->get_gameObject(), "Whether or not your drawing is visible during gameplay");
            
            // if post processing enabled, allow to turn it off for scribble
            auto effectContainers = Resources::FindObjectsOfTypeAll<GlobalNamespace::MainEffectContainerSO*>();
            if (effectContainers && effectContainers.Length() > 0)
            {
                auto effectContainer = effectContainers[0];
                if (effectContainer->postProcessEnabled->get_value())
                {
                    toggle = BeatSaberUI::CreateToggle(container->get_transform(), "Use Real Glow", config.useRealGlow, [](bool val){ 
                        config.useRealGlow = val; 
                        SaveConfig();
                        ScribbleContainer::SetRealGlow(val);
                    });
                    BeatSaberUI::AddHoverHint(toggle->get_gameObject(), "Whether the mod uses real glow or just makes lines more white when glow is high");
                }
            }
        }
    }
}