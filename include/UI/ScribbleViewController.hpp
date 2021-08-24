#pragma once

#include "custom-types/shared/macros.hpp"
#include "UnityEngine/Color.hpp"
#include "HMUI/ViewController.hpp"
#include "HMUI/ModalView.hpp"

DECLARE_CLASS_CODEGEN(Scribble, ScribbleViewController, HMUI::ViewController,
    DECLARE_OVERRIDE_METHOD(void, DidActivate, il2cpp_utils::FindMethodUnsafe("HMUI", "ViewController", "DidActivate", 3), bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling);
    DECLARE_INSTANCE_FIELD(HMUI::ModalView*, colorPickerModal);

    public:
        int size = 1;
        float glow = 0.5f;
        void SelectBrush(int idx);
        void SelectTexture(int idx);
        void SelectEffect(int idx);
        void SelectPicker();
        void PickerSelectedColor(UnityEngine::Color color);
        void SelectEraseMode();
)