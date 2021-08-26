#pragma once

#include "custom-types/shared/macros.hpp"
#include "questui/shared/CustomTypes/Components/List/CustomListTableData.hpp"
#include "UnityEngine/Color.hpp"
#include "UnityEngine/UI/Image.hpp"
#include "HMUI/ViewController.hpp"
#include "HMUI/ModalView.hpp"
#include "HMUI/InputFieldView.hpp"
#include "questui/shared/CustomTypes/Components/Settings/SliderSetting.hpp"
#include "questui/shared/CustomTypes/Components/ModalColorPicker.hpp"
#include "CustomTypes/BrushBehaviour.hpp"

DECLARE_CLASS_CODEGEN(Scribble, ScribbleViewController, HMUI::ViewController,
    DECLARE_OVERRIDE_METHOD(void, DidActivate, il2cpp_utils::FindMethodUnsafe("HMUI", "ViewController", "DidActivate", 3), bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling);
    DECLARE_OVERRIDE_METHOD(void, DidDeactivate, il2cpp_utils::FindMethodUnsafe("HMUI", "ViewController", "DidDeactivate", 2), bool, bool);
    DECLARE_INSTANCE_FIELD(QuestUI::ModalColorPicker*, colorPickerModal);
    DECLARE_INSTANCE_FIELD(QuestUI::CustomListTableData*, brushList);
    DECLARE_INSTANCE_FIELD(QuestUI::CustomListTableData*, textureList);
    DECLARE_INSTANCE_FIELD(QuestUI::CustomListTableData*, effectList);
    DECLARE_INSTANCE_FIELD(QuestUI::CustomListTableData*, saveFileList);
    DECLARE_INSTANCE_FIELD(QuestUI::CustomListTableData*, loadFileList);
    DECLARE_INSTANCE_FIELD(QuestUI::SliderSetting*, glowSlider);
    DECLARE_INSTANCE_FIELD(QuestUI::SliderSetting*, sizeSlider);
    DECLARE_INSTANCE_FIELD(QuestUI::SliderSetting*, tileSlider);
    DECLARE_INSTANCE_FIELD(HMUI::InputFieldView*, fileNameField);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Image*, eraserImage);
    
    DECLARE_INSTANCE_FIELD(HMUI::ModalView*, saveModal);
    DECLARE_INSTANCE_FIELD(HMUI::ModalView*, loadModal);

    private:
        void ReloadBrushList();
        void ReloadTextureList();
        void ReloadEffectList();
        void ReloadFileLists();
        void SelectForBrush(const CustomBrush& brush);
        void ActiveControllerChanged(BrushBehaviour* newBrush);
        void SetModalPosition(HMUI::ModalView* modal);
        void SaveSelectIdx(int idx);
        
        std::string saveFileName = "";
    public:
        int size = 1;
        float glow = 0.5f;
        float tile = 1.0f;
        void SelectBrush(int idx);
        void SelectTexture(int idx);
        void PickerSelectedColor(UnityEngine::Color color);
        void SelectEffect(int idx);
        void SelectPicker();
        void SelectEraseMode();
        void ShowSaveFile();
        void ShowLoadFile();
        void SaveFilenameChanged(std::string val);
)