#include "UI/ColorHistoryPanelController.hpp"

#include <utility>

#include "UnityEngine/Resources.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Object.hpp"
#include "UnityEngine/RectTransform.hpp"
#include "UnityEngine/UI/LayoutElement.hpp"
#include "System/Action_1.hpp"
#include "questui/shared/BeatSaberUI.hpp"
#include "questui/shared/ArrayUtil.hpp"

DEFINE_TYPE(Scribble, ColorHistoryPanelController);

using namespace UnityEngine;
using namespace QuestUI;

namespace Scribble
{
    void ColorHistoryPanelController::ctor()
    {
        INVOKE_CTOR();
    }

    ColorHistoryPanelController* ColorHistoryPanelController::CreateColorHistoryPanel(UnityEngine::Transform* parent, UnityEngine::Vector2 sizeDelta, std::function<void(Sombrero::FastColor)> onColorSelected)
    {
        auto horizontal = BeatSaberUI::CreateHorizontalLayoutGroup(parent->get_transform());
        horizontal->get_rectTransform()->set_anchoredPosition({15, 0});
        horizontal->set_childForceExpandWidth(false);
        horizontal->set_spacing(1);
        horizontal->GetComponent<UI::LayoutElement*>()->set_preferredWidth(54);
        auto panelController = horizontal->get_gameObject()->AddComponent<ColorHistoryPanelController*>();
        panelController->onColorSelected = std::move(onColorSelected);
        return panelController;
    }

    void ColorHistoryPanelController::AddColor(Sombrero::FastColor color)
    {
        if (!controllerPanelTemplate) controllerPanelTemplate = ArrayUtil::First(Resources::FindObjectsOfTypeAll<GlobalNamespace::PreviousColorPanelController*>(), [](auto x){ return to_utf8(csstrtostr(x->get_name())) == "PreviousColorPanelController";});
        if (!colorPanelHistory) colorPanelHistory = List<GlobalNamespace::PreviousColorPanelController*>::New_ctor();

        // if color already in history, return
        for (int i = 0; i < colorPanelHistory->get_Count(); i++)
            if (colorPanelHistory->items->values[i]->dyn__color() == color) return;

        // we are adding when there isn't max amount of history objects yet
        if (colorPanelHistory->get_Count() < maxHistoryLength)
        {
            auto newController = CreatePreviousColorController();
            // ew ugly color and addcolor, but due to how the component works, this is what we will do
            newController->dyn__color() = color;
            newController->AddColor(color);
            colorPanelHistory->Add(newController);
        }
        // enough history already
        else
        {
            for (int i = 0; i < maxHistoryLength - 1; i++)
            {
                colorPanelHistory->items->values[i]->dyn__color() = colorPanelHistory->items->values[i + 1]->dyn__color();
                colorPanelHistory->items->values[i]->AddColor(colorPanelHistory->items->values[i + 1]->dyn__color());
            }
            
            colorPanelHistory->items->values[maxHistoryLength - 1]->dyn__color() = color;
            colorPanelHistory->items->values[maxHistoryLength - 1]->AddColor(color);
        }
    }

    GlobalNamespace::PreviousColorPanelController* ColorHistoryPanelController::CreatePreviousColorController()
    {
        auto wrapper = GameObject::New_ctor()->AddComponent<RectTransform*>();
        wrapper->SetParent(get_transform(), false);
        auto layout = wrapper->get_gameObject()->AddComponent<UI::LayoutElement*>();
        layout->set_preferredWidth(10);
        auto gameObject = Object::Instantiate(controllerPanelTemplate->get_gameObject(), wrapper->get_transform());

        auto panel = gameObject->GetComponent<GlobalNamespace::PreviousColorPanelController*>();
        std::function<void(UnityEngine::Color)> fun = [this](auto && PH1) { OnColorSelected(std::forward<decltype(PH1)>(PH1)); };
        auto delegate = il2cpp_utils::MakeDelegate<System::Action_1<UnityEngine::Color>*>(classof(System::Action_1<UnityEngine::Color>*), fun);
        panel->add_colorWasSelectedEvent(delegate);
        wrapper->SetAsFirstSibling();

        return panel;
    }

    void ColorHistoryPanelController::OnColorSelected(UnityEngine::Color color) const
    {
        if (onColorSelected) onColorSelected(color);
    }
}