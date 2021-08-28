#pragma once

#include "custom-types/shared/macros.hpp"
#include "GlobalNamespace/PreviousColorPanelController.hpp"
#include "UnityEngine/Vector2.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/Color.hpp"

#include "sombrero/shared/ColorUtils.hpp"

#include <functional>

DECLARE_CLASS_CODEGEN(Scribble, ColorHistoryPanelController, UnityEngine::MonoBehaviour,
    DECLARE_INSTANCE_FIELD(List<GlobalNamespace::PreviousColorPanelController*>*, colorPanelHistory);
    DECLARE_INSTANCE_FIELD(GlobalNamespace::PreviousColorPanelController*, controllerPanelTemplate);

    DECLARE_CTOR(ctor);
    public:
        static ColorHistoryPanelController* CreateColorHistoryPanel(UnityEngine::Transform* parent, UnityEngine::Vector2 sizeDelta, std::function<void(Sombrero::FastColor)> onColorSelected);
        GlobalNamespace::PreviousColorPanelController* CreatePreviousColorController();
        int maxHistoryLength = 5;
        std::function<void(Sombrero::FastColor)> onColorSelected = nullptr;
        void AddColor(Sombrero::FastColor color);
        void OnColorSelected(UnityEngine::Color color);
)