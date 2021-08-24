#pragma once

#include "UIElements.hpp"
#include "UnityEngine/Vector2.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/Resources.hpp"
#include "UnityEngine/UI/ContentSizeFitter.hpp"
#include "UnityEngine/UI/Image.hpp"
#include "questui/shared/ArrayUtil.hpp"
#include "TMPro/TextMeshProUGUI.hpp"
#include "VRUIControls/VRGraphicRaycaster.hpp"

#include <string_view>

namespace UITools
{
    TMPro::TextMeshProUGUI* CreateText(UnityEngine::RectTransform* parent, std::string_view text, UnityEngine::Vector2 anchoredPosition);

    void SetRaycasterCamera(UnityEngine::GameObject* go, UnityEngine::Canvas* canvas = nullptr);

    UnityEngine::UI::Image* CreateImage(UnityEngine::Transform* parent, UnityEngine::Vector2 anchoredPosition, UnityEngine::Vector2 sizeDelta);
    
    UnityEngine::GameObject* CreateFromInstance(UnityEngine::Transform* parent, std::string_view name, bool worldPositionStays = false);

    Scribble::ScribbleUISlider CreateSlider(UnityEngine::Transform* parent);

    Scribble::ScribbleUISimpleButton CreateSimpleButton(UnityEngine::Transform* parent, std::string_view text);

    Scribble::ScribbleUICheckbox CreateCheckbox(UnityEngine::Transform* parent);

    UnityEngine::GameObject* GetChild(UnityEngine::GameObject* go, std::string_view path);

    template<typename T>
    requires(std::is_convertible_v<T, UnityEngine::Component*>)
    T GetComponent(UnityEngine::GameObject* go, std::string_view path)
    {
        return GetChild(go, path)->GetComponent<T>();
    }
}