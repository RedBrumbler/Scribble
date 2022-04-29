#pragma once

#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "UnityEngine/Object.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/RectTransform.hpp"
#include "UnityEngine/Vector2.hpp"
#include "UnityEngine/UI/Toggle.hpp"
#include "UnityEngine/UI/Toggle_ToggleEvent.hpp"
#include "UnityEngine/UI/Button.hpp"
#include "UnityEngine/UI/Button_ButtonClickedEvent.hpp"
#include "UnityEngine/Events/UnityAction_1.hpp"
#include "UnityEngine/Events/UnityAction.hpp"

#include "HMUI/RangeValuesTextSlider.hpp"
#include "HMUI/NoTransitionsButton.hpp"

#include "TMPro/TextMeshProUGUI.hpp"
#include "System/Action_2.hpp"
#include "Polyglot/LocalizedTextMeshProUGUI.hpp"

namespace Scribble
{
    class ScribbleUIElement
    {
        public:
        UnityEngine::GameObject* gameObject;
        UnityEngine::RectTransform* rectTransform;

        explicit ScribbleUIElement(UnityEngine::GameObject* go) : gameObject(go), rectTransform(go->GetComponent<UnityEngine::RectTransform*>()) {};

        void SetAnchor(float x, float y)
        {
            SetAnchor(x, y, x, y);
        }

        void SetAnchor(float minX, float minY, float maxX, float maxY) const
        {
            rectTransform->set_anchorMin({minX, minY});
            rectTransform->set_anchorMax({maxX, maxY});
        }

        void SetPosition(float x, float y) const
        {
            rectTransform->set_anchoredPosition({x, y});
        }

        void SetSize(float x, float y) const
        {
            rectTransform->set_sizeDelta({x, y});
        }
    };

    class ScribbleUISlider : public ScribbleUIElement
    {
        public:
            HMUI::RangeValuesTextSlider* slider;
            TMPro::TextMeshProUGUI* textMesh;

            bool get_enableDragging() const
            {
                return slider->dyn__enableDragging();
            }

            void set_enableDragging(bool value) const
            {
                slider->dyn__enableDragging() = value;
            }

            void set_text(std::string_view value) const
            {
                textMesh->set_text(value);
            }

            explicit ScribbleUISlider(UnityEngine::GameObject* go) : ScribbleUIElement(go), slider(go->GetComponentInChildren<HMUI::RangeValuesTextSlider*>()), textMesh(go->GetComponentInChildren<TMPro::TextMeshProUGUI*>()) {};

            void SetRange(float min, float max) const
            {
                slider->set_minValue(min);
                slider->set_maxValue(max);
            }

            void SetValue(float val) const
            {
                slider->set_value(val);
            }

            void AddListener(std::function<void(HMUI::RangeValuesTextSlider*, float)> fun) const
            {
                slider->add_valueDidChangeEvent(il2cpp_utils::MakeDelegate<System::Action_2<HMUI::RangeValuesTextSlider*, float>*>(classof(System::Action_2<HMUI::RangeValuesTextSlider*, float>*), fun));
            }
    };

    class ScribbleUISimpleButton : public ScribbleUIElement
    {
        public:
            HMUI::NoTransitionsButton* noTransitionsButton;
            TMPro::TextMeshProUGUI* textMesh;
            bool strokeEnabled{};

            void set_text(std::string_view value) const
            {
                textMesh->set_text(value);
            }

            explicit ScribbleUISimpleButton(UnityEngine::GameObject* go, std::string_view text = "button") : ScribbleUIElement(go), textMesh(go->GetComponentInChildren<TMPro::TextMeshProUGUI*>()), noTransitionsButton(go->GetComponent<HMUI::NoTransitionsButton*>())
            {
                go->set_name(text);
                UnityEngine::Object::DestroyImmediate(go->GetComponentInChildren<Polyglot::LocalizedTextMeshProUGUI*>());
                textMesh->set_text(text);
            }

            void AddListener(std::function<void()> fun) const
            {
                noTransitionsButton->get_onClick()->AddListener(il2cpp_utils::MakeDelegate<UnityEngine::Events::UnityAction*>(classof(UnityEngine::Events::UnityAction*), fun));
            }
    };

    class ScribbleUICheckbox : public ScribbleUIElement
    {
        public:
            UnityEngine::UI::Toggle* toggle;
            TMPro::TextMeshProUGUI* textMesh;

            [[maybe_unused]] void set_text(std::string_view value) const
            {
                textMesh->set_text(value);
            }

            explicit ScribbleUICheckbox(UnityEngine::GameObject* go) : ScribbleUIElement(go), toggle(go->GetComponentInChildren<UnityEngine::UI::Toggle*>()), textMesh(go->GetComponentInChildren<TMPro::TextMeshProUGUI*>()) {};

            void AddListener(std::function<void(bool)> fun) const
            {
                toggle->dyn_onValueChanged()->AddListener(il2cpp_utils::MakeDelegate<UnityEngine::Events::UnityAction_1<bool>*>(classof(UnityEngine::Events::UnityAction_1<bool>*), fun));
            }
    };
}