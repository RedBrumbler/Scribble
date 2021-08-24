#pragma once

#include "custom-types/shared/macros.hpp"
#include "questui/shared/CustomTypes/Components/List/CustomListTableData.hpp"
#include "UnityEngine/Color.hpp"
#include "UnityEngine/UI/Image.hpp"
#include "HMUI/ViewController.hpp"
#include "HMUI/ModalView.hpp"
#include "questui/shared/CustomTypes/Components/Settings/IncrementSetting.hpp"

DECLARE_CLASS_CODEGEN(Scribble, ScribbleViewController, HMUI::ViewController,
    DECLARE_OVERRIDE_METHOD(void, DidActivate, il2cpp_utils::FindMethodUnsafe("HMUI", "ViewController", "DidActivate", 3), bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling);
    DECLARE_INSTANCE_FIELD(HMUI::ModalView*, colorPickerModal);
    DECLARE_INSTANCE_FIELD(QuestUI::CustomListTableData*, brushList);
    DECLARE_INSTANCE_FIELD(QuestUI::CustomListTableData*, textureList);
    DECLARE_INSTANCE_FIELD(QuestUI::CustomListTableData*, effectList);
    DECLARE_INSTANCE_FIELD(QuestUI::IncrementSetting*, glowSlider);
    DECLARE_INSTANCE_FIELD(QuestUI::IncrementSetting*, sizeSlider);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Image*, eraserImage);
    
    private:
        void ReloadBrushList();
        void ReloadTextureList();
        void ReloadEffectList();

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