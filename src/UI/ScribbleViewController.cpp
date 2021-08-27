#include "UI/ScribbleViewController.hpp"
#include "UI/UITools.hpp"
#include "ScribbleContainer.hpp"
#include "Brushes.hpp"
#include "Effects.hpp"
#include "BrushTextures.hpp"
#include "GlobalBrushManager.hpp"
#include "UnityEngine/Sprite.hpp"
#include "UnityEngine/Rect.hpp"
#include "UnityEngine/Vector2.hpp"
#include "UnityEngine/Vector4.hpp"
#include "UnityEngine/RectOffset.hpp"
#include "UnityEngine/UI/ContentSizeFitter.hpp"
#include "UnityEngine/SpriteMeshType.hpp"
#include "logging.hpp"

#include "HMUI/TableView_ScrollPositionType.hpp"
#include "questui/shared/BeatSaberUI.hpp"
#include "questui/shared/CustomTypes/Components/List/QuestUITableView.hpp"
#include "questui/shared/CustomTypes/Components/Backgroundable.hpp"
#include "static-defines.hpp"
#include "icons.hpp"

#include "System/Collections/Generic/HashSet_1.hpp"

#include "Utils/FileUtils.hpp"
#include "Utils/ThumbnailHelper.hpp"

DEFINE_TYPE(Scribble, ScribbleViewController);

using namespace QuestUI;
using namespace UnityEngine;
using namespace UnityEngine::UI;

namespace Scribble
{
    std::string ScribbleViewController::FindNextName(std::string_view name, int idx)
    {
        // candidate, what we think might work
        std::string candidate = idx == 0 ? std::string(name) : string_format("%s-%d", name.data(), idx);
        // check all existing brushes for this name
        for (auto b : saveBrushList->data)
        {
            // if we find the name, increment idx by 1 and try again
            if (b.text == candidate) return FindNextName(name, ++idx);
        }
        // if we didnt find the name, its good, return it
        return candidate;
    }

    void ScribbleViewController::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling)
    {
        if (firstActivation)
        {
            // going to attempt to port the UI from BSML
            
            //<vertical anchor-pos-y="-25" child-expand-height="false" child-control-height="false" spacing="2" xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance' xsi:schemaLocation='https://monkeymanboy.github.io/BSML-Docs/ https://raw.githubusercontent.com/monkeymanboy/BSML-Docs/gh-pages/BSMLSchema.xsd'>
            {
            auto vertical = BeatSaberUI::CreateVerticalLayoutGroup(get_transform());
            vertical->get_gameObject()->GetComponent<Backgroundable*>()->ApplyBackgroundWithAlpha(il2cpp_utils::newcsstr("round-rect-panel"), 0.5f);
            vertical->get_rectTransform()->set_anchoredPosition({0.0f, -25.0f});
            vertical->set_childForceExpandHeight(false);
            vertical->set_childControlHeight(false);
            vertical->set_spacing(2);
                //<horizontal horizontal-fit="PreferredSize" child-expand-width="false" child-expand-height="true" child-control-width="true" child-control-height="true" spacing="5">
                {
                auto horizontal = BeatSaberUI::CreateHorizontalLayoutGroup(vertical->get_transform());
                auto sizeFitter = horizontal->get_gameObject()->AddComponent<ContentSizeFitter*>();
                sizeFitter->set_horizontalFit(ContentSizeFitter::FitMode::PreferredSize);
                horizontal->set_childForceExpandWidth(false);
                horizontal->set_childForceExpandHeight(true);
                horizontal->set_childControlWidth(true);
                horizontal->set_childControlHeight(true);
                horizontal->set_spacing(5);
                    //<vertical pad-top="5">
                    {
                        auto verticalTop = BeatSaberUI::CreateVerticalLayoutGroup(horizontal->get_transform());
                        verticalTop->set_padding(UnityEngine::RectOffset::New_ctor(0, 0, 5, 0));
                        verticalTop->set_spacing(2);
                        auto layoutTop = verticalTop->get_gameObject()->AddComponent<LayoutElement*>();
                        layoutTop->set_preferredWidth(12);
                        verticalTop->set_childControlHeight(true);
                        verticalTop->set_childControlWidth(true);
                        verticalTop->set_childForceExpandHeight(false);
                        verticalTop->set_childForceExpandWidth(false);
                        
                        //<vertical spacing="2" preferred-width="12" child-control-height="true" child-control-width="true" child-expand-height="false" child-expand-width="false">
                        {
                            auto eraserPickerVertical = BeatSaberUI::CreateVerticalLayoutGroup(verticalTop->get_transform());
                            auto layoutButtons = verticalTop->get_gameObject()->AddComponent<LayoutElement*>();
                            eraserPickerVertical->GetComponent<ContentSizeFitter*>()->set_verticalFit(ContentSizeFitter::FitMode::PreferredSize);
                            float buttonSize = 13.5;
                            eraserPickerVertical->set_spacing(2);
                            layoutButtons->set_preferredWidth(buttonSize);
                            eraserPickerVertical->set_childControlHeight(true);
                            eraserPickerVertical->set_childControlWidth(true);
                            eraserPickerVertical->set_childForceExpandHeight(false);
                            eraserPickerVertical->set_childForceExpandWidth(false);

                            auto pickerButton = BeatSaberUI::CreateUIButton(eraserPickerVertical->get_transform(), "", "SettingsButton", Vector2(0, 0), Vector2(buttonSize, buttonSize), std::bind(&ScribbleViewController::SelectPicker, this));
                            reinterpret_cast<RectTransform*>(pickerButton->get_transform()->GetChild(0))->set_sizeDelta({buttonSize, buttonSize});
                            pickerImage = pickerButton->GetComponentInChildren<HMUI::ImageView*>();
                            
                            //<button-with-icon id="picker-btn" stroke-type="Clean" preferred-width="12" preferred-height="12" on-click="selectPicker" click-event="show-picker" hover-hint="Color Picker" />
                            auto eraserButton = BeatSaberUI::CreateUIButton(eraserPickerVertical->get_transform(), "", "SettingsButton", Vector2(0, 0), Vector2(buttonSize, buttonSize), std::bind(&ScribbleViewController::SelectEraseMode, this));
                            reinterpret_cast<RectTransform*>(eraserButton->get_transform()->GetChild(0))->set_sizeDelta({buttonSize, buttonSize});
                            eraserImage = eraserButton->GetComponentInChildren<HMUI::ImageView*>();
                            
                            auto bucketButton = BeatSaberUI::CreateUIButton(eraserPickerVertical->get_transform(), "", "SettingsButton", Vector2(0, 0), Vector2(buttonSize, buttonSize), std::bind(&ScribbleViewController::SelectBucketMode, this));
                            reinterpret_cast<RectTransform*>(bucketButton->get_transform()->GetChild(0))->set_sizeDelta({buttonSize, buttonSize});
                            bucketImage = bucketButton->GetComponentInChildren<HMUI::ImageView*>();

                            auto brushSaveButton = BeatSaberUI::CreateUIButton(eraserPickerVertical->get_transform(), "", "SettingsButton", Vector2(0, 0), Vector2(buttonSize, buttonSize), [&](){
                                brushModal->Show(true, true, nullptr);
                            });
                            reinterpret_cast<RectTransform*>(brushSaveButton->get_transform()->GetChild(0))->set_sizeDelta({buttonSize, buttonSize});

                            BeatSaberUI::SetButtonSprites(pickerButton, UITools::Base64ToSprite(picker_inactive), UITools::Base64ToSprite(picker));
                            BeatSaberUI::SetButtonSprites(eraserButton, UITools::Base64ToSprite(eraser_inactive), UITools::Base64ToSprite(eraser));
                            BeatSaberUI::SetButtonSprites(bucketButton, UITools::Base64ToSprite(bucket_inactive), UITools::Base64ToSprite(bucket));
                            BeatSaberUI::SetButtonSprites(brushSaveButton, UITools::Base64ToSprite(save_inactive), UITools::Base64ToSprite(save));
                            
                            /*
                            Il2CppString* practiceButton_cs = il2cpp_utils::newcsstr("PracticeButton");
                            auto orig = QuestUI::ArrayUtil::Last(Resources::FindObjectsOfTypeAll<Button*>(),  [&](Button* x) {
                                return x->get_name()->Equals(practiceButton_cs);
                            });
                            auto material = orig->get_gameObject()->GetComponentInChildren<HMUI::ImageView*>()->get_material();
                            std::string pickerPath = "/sdcard/ModData/com.beatgames.beatsaber/Mods/Scribble/picker.png";
                            auto sprite = BeatSaberUI::FileToSprite(pickerPath);
                            auto img = UITools::CreateImage(pickerButton->get_transform(), {0, 0}, {12, 12});
                            img->set_material(material);
                            img->set_sprite(sprite);

                            std::string eraserPath = "/sdcard/ModData/com.beatgames.beatsaber/Mods/Scribble/eraser.png";
                            sprite = BeatSaberUI::FileToSprite(eraserPath);
                            eraserImage = UITools::CreateImage(eraserButton->get_transform(), {0, 0}, {12, 12});
                            eraserImage->set_material(material);
                            eraserImage->set_sprite(sprite);
                            */
                            //<button-with-icon id="eraser-btn" stroke-type="Clean" preferred-width="12" preferred-height="12" on-click="selectEraseMode" hover-hint="Eraser" />
                        }
                        //  </vertical>
                    }
                    //</vertical>

                    UnityEngine::Color color = {0, 0, 0, 1.0};
                    if (GlobalBrushManager::get_activeBrush()) color = GlobalBrushManager::get_activeBrush()->currentBrush.color;
                    colorPickerModal = BeatSaberUI::CreateColorPickerModal(get_transform(), "", color, std::bind(&ScribbleViewController::PickerSelectedColor, this, std::placeholders::_1));
                    
                    //<modal id='save-dialog' hide-event='save-dialog-hide' move-to-center="true" click-off-closes="true" size-delta-x="70" size-delta-y="80">
                    saveModal = BeatSaberUI::CreateModal(horizontal->get_transform(), Vector2(80, 80), nullptr, true);
                    {
                        //  <vertical spacing="8" pad="5">
                        auto saveHorizontal = BeatSaberUI::CreateHorizontalLayoutGroup(saveModal->get_transform());
                        saveHorizontal->set_childForceExpandWidth(false);
                        saveHorizontal->set_childForceExpandHeight(true);

                            saveFileList = BeatSaberUI::CreateScrollableList(saveHorizontal->get_transform(), Vector2(30, 76), std::bind(&ScribbleViewController::SaveSelectIdx, this, std::placeholders::_1));
                            saveFileList->set_listStyle(QuestUI::CustomListTableData::ListStyle::Box);
                            saveFileList->cellSize = 30.0f;

                            auto saveInfoVertical = BeatSaberUI::CreateVerticalLayoutGroup(saveHorizontal->get_transform());
                            saveInfoVertical->get_gameObject()->AddComponent<QuestUI::Backgroundable*>()->ApplyBackgroundWithAlpha(il2cpp_utils::newcsstr("round-rect-panel"), 0.2f);
                            saveInfoVertical->set_padding(UnityEngine::RectOffset::New_ctor(0, 0, 20, 20));
                            saveInfoVertical->get_gameObject()->AddComponent<LayoutElement*>()->set_preferredHeight(40);
                            saveInfoVertical->set_childForceExpandHeight(false);
                                auto saveFileNameHorizontal = BeatSaberUI::CreateHorizontalLayoutGroup(saveInfoVertical->get_transform());
                                saveFileNameHorizontal->set_childForceExpandHeight(false);
                                
                                fileNameField = BeatSaberUI::CreateStringSetting(saveFileNameHorizontal->get_transform(), "filename", "", std::bind(&ScribbleViewController::SaveFilenameChanged, this, std::placeholders::_1) );
                                auto nameFieldLayout = fileNameField->GetComponent<LayoutElement*>();
                                nameFieldLayout->set_preferredWidth(30);

                                auto saveConfirmHorizontal = BeatSaberUI::CreateHorizontalLayoutGroup(saveInfoVertical->get_transform());
                                saveConfirmHorizontal->set_childForceExpandHeight(false);
                                auto saveClose = BeatSaberUI::CreateUIButton(saveConfirmHorizontal->get_transform(), "Close", [&](){ 
                                    saveModal->Hide(true, nullptr);
                                    saveFileName = "";
                                    fileNameField->set_text(Il2CppString::_get_Empty());
                                    ReloadFileLists();
                                });
                                auto saveNew = BeatSaberUI::CreateUIButton(saveConfirmHorizontal->get_transform(), "Save", [&](){ 
                                    ScribbleContainer::get_instance()->Save(string_format("%s/%s.png", drawingPath, saveFileName.c_str()));
                                });
                    }
                    loadModal = BeatSaberUI::CreateModal(horizontal->get_transform(), Vector2(80, 80), nullptr, true);
                    {
                        auto loadHorizontal = BeatSaberUI::CreateHorizontalLayoutGroup(loadModal->get_transform());
                        loadHorizontal->set_childForceExpandWidth(false);
                        loadHorizontal->set_childForceExpandHeight(true);

                        loadFileList = BeatSaberUI::CreateScrollableList(loadHorizontal->get_transform(), Vector2(30, 76));
                        loadFileList->set_listStyle(QuestUI::CustomListTableData::ListStyle::Box);
                        loadFileList->cellSize = 30.0f;

                            auto loadInfoVertical = BeatSaberUI::CreateVerticalLayoutGroup(loadHorizontal->get_transform());
                            loadInfoVertical->get_gameObject()->AddComponent<QuestUI::Backgroundable*>()->ApplyBackgroundWithAlpha(il2cpp_utils::newcsstr("round-rect-panel"), 0.2f); 
                            loadInfoVertical->get_gameObject()->AddComponent<LayoutElement*>()->set_preferredHeight(40);
                            loadInfoVertical->set_childForceExpandHeight(false);
                            loadInfoVertical->set_padding(UnityEngine::RectOffset::New_ctor(0, 0, 20, 20));

                            auto loadClose = BeatSaberUI::CreateUIButton(loadInfoVertical->get_transform(), "Close", [&](){ loadModal->Hide(true, nullptr); });
                            auto loadLoad = BeatSaberUI::CreateUIButton(loadInfoVertical->get_transform(), "Load", [&](){ 
                                int idx = reinterpret_cast<QuestUI::TableView*>(loadFileList->tableView)->get_selectedRow();
                                if (idx < 0) return;
                                //loadFileList->tableView->selectedCellIdxs->items->values[0];
                                ScribbleContainer::get_instance()->Load(string_format("%s/%s.png", drawingPath, loadFileList->data[idx].text.c_str()));
                                loadFileList->tableView->selectedCellIdxs->Clear();
                                loadFileList->tableView->ReloadData();
                            });
                    }
                    ReloadFileLists();
                    
                    brushModal = BeatSaberUI::CreateModal(horizontal->get_transform(), Vector2(80, 80), [&](auto){
                        // clear selection on menu leave
                        saveBrushList->tableView->ClearSelection();
                    }, true);
                    {
                        auto brushHorizontal = BeatSaberUI::CreateHorizontalLayoutGroup(brushModal->get_transform());
                        brushHorizontal->set_childForceExpandWidth(false);
                        brushHorizontal->set_childForceExpandHeight(true);

                        saveBrushList = BeatSaberUI::CreateScrollableList(brushHorizontal->get_transform(), Vector2(35, 60), [&](int idx){
                            brushName = saveBrushList->data[idx].text;
                            brushNameField->set_text(il2cpp_utils::newcsstr(brushName));
                        });

                        auto brushSaveButtonVertical = BeatSaberUI::CreateVerticalLayoutGroup(brushHorizontal->get_transform());
                        brushSaveButtonVertical->set_padding(RectOffset::New_ctor(0, 0, 20, 20));
                        // what do we want to do with brushes? 
                        // - delete
                        // - save current setup -> name input
                        // - duplicate selected in save menu
                        //auto brushNameFieldHorizontal = 
                        brushNameField = BeatSaberUI::CreateStringSetting(brushSaveButtonVertical->get_transform(), "brush name", "", std::bind(&ScribbleViewController::BrushNameChanged, this, std::placeholders::_1) );
                        auto brushNameFieldLayout = brushNameField->GetComponent<LayoutElement*>();
                        brushNameFieldLayout->set_preferredWidth(30);
                        auto brushSaveButtonHorizontal = BeatSaberUI::CreateHorizontalLayoutGroup(brushSaveButtonVertical->get_transform());
                        brushSaveButtonHorizontal->GetComponent<ContentSizeFitter*>()->set_horizontalFit(ContentSizeFitter::FitMode::PreferredSize);
                        float brushButtonSize = 10.0f;
                        brushSaveButtonHorizontal->set_spacing(2);
                        brushSaveButtonHorizontal->set_childForceExpandWidth(false);
                        brushSaveButtonHorizontal->set_childForceExpandHeight(false);
                        brushSaveButtonHorizontal->set_childControlHeight(false);
                        brushSaveButtonHorizontal->set_childControlWidth(false);
                        auto saveButtonLayout = brushSaveButtonHorizontal->get_gameObject()->AddComponent<LayoutElement*>();
                        saveButtonLayout->set_preferredHeight(brushButtonSize);
                        saveButtonLayout->set_preferredWidth(brushButtonSize * 3 + 4);

                        auto deleteButton = BeatSaberUI::CreateUIButton(brushSaveButtonHorizontal->get_transform(), "", "SettingsButton", Vector2(0, 0), Vector2(brushButtonSize, brushButtonSize), [&](){
                            int idx = reinterpret_cast<QuestUI::TableView*>(saveBrushList->tableView)->get_selectedRow();
                            INFO("brush delete idx: %d", idx);
                            if (idx < 0) return;
                            auto& selectedBrush = saveBrushList->data[idx];
                            auto itr = std::find_if(Brushes::brushes.begin(), Brushes::brushes.end(), [&](const auto& x){ return x.name == selectedBrush.text; });
                            if (itr != Brushes::brushes.end())
                            {
                                INFO("erasing brush: %s", selectedBrush.text.c_str());
                                Brushes::brushes.erase(itr, itr + 1);
                                Brushes::Save();
                            }

                            ReloadBrushList();
                        });
                        
                        auto copyButton = BeatSaberUI::CreateUIButton(brushSaveButtonHorizontal->get_transform(), "", "SettingsButton", Vector2(0, 0), Vector2(brushButtonSize, brushButtonSize), [&](){
                            int idx = reinterpret_cast<QuestUI::TableView*>(saveBrushList->tableView)->get_selectedRow();
                            INFO("brush copy idx: %d", idx);
                            if (idx < 0) return;
                            auto& selectedBrush = saveBrushList->data[idx];
                            auto brush = Brushes::GetBrush(selectedBrush.text);
                            if (brush)
                            {
                                INFO("Got a brush with name: %s", selectedBrush.text.c_str());
                                // the brush in our managed list
                                auto& brushRef = brush.value().get();
                                // duplicate that one
                                CustomBrush duplicate = brushRef;

                                // if no given name, copy the original name
                                if (brushName == "") brushName = FindNextName(brushRef.name, 0);
                                // if a given name, use that instead
                                else brushName = brushName = FindNextName(brushName, 0);

                                // set name
                                duplicate.name = brushName;
                                Brushes::brushes.push_back(duplicate);
                                // clear name setting
                                brushName = "";
                                brushNameField->set_text(Il2CppString::_get_Empty());
                            }

                            ReloadBrushList();
                        });

                        auto saveButton = BeatSaberUI::CreateUIButton(brushSaveButtonHorizontal->get_transform(), "", "SettingsButton", Vector2(0, 0), Vector2(brushButtonSize, brushButtonSize), [&](){
                            int idx = reinterpret_cast<QuestUI::TableView*>(saveBrushList->tableView)->get_selectedRow();
                            if (idx < 0) return;
                            // we have an index that is selected!;
                            std::string selectedBrushName = idx >= 0 ? saveBrushList->data[idx].text : "";

                            INFO("saving idx: %d", idx);

                            // if we want to save, and we have a brush name
                            if (selectedBrushName != "")
                            {
                                auto brush = Brushes::GetBrush(selectedBrushName);
                                // if the brush existed in the first place
                                if (brush)
                                {
                                    auto& brushRef = brush.value().get();
                                    brushRef.copy(GlobalBrushManager::get_activeBrush()->currentBrush);
                                    if (brushRef.name != brushName) brushRef.name = brushName;
                                }
                            } 
                            // we want to save a brush, but have no brush to overwrite
                            else if (brushName != "")
                            {
                                auto brush = Brushes::GetBrush(brushName);
                                if (brush)
                                {
                                    // if this name already exists
                                    brushName = FindNextName(brushName, 0);
                                    CustomBrush duplicate = brush.value().get();
                                    duplicate.name = brushName;
                                    Brushes::brushes.push_back(duplicate);
                                }
                                else
                                {
                                    // new brush!
                                    CustomBrush newBrush = GlobalBrushManager::get_activeBrush()->currentBrush;
                                    newBrush.name = brushName;
                                    Brushes::brushes.push_back(newBrush);
                                }
                            }
                            Brushes::Save();

                            brushName = "";
                            brushNameField->set_text(Il2CppString::_get_Empty());
                            ReloadBrushList();
                        });
                        reinterpret_cast<RectTransform*>(deleteButton->get_transform()->GetChild(0))->set_sizeDelta({ brushButtonSize, brushButtonSize });
                        reinterpret_cast<RectTransform*>(copyButton->get_transform()->GetChild(0))->set_sizeDelta({ brushButtonSize, brushButtonSize });
                        reinterpret_cast<RectTransform*>(saveButton->get_transform()->GetChild(0))->set_sizeDelta({ brushButtonSize, brushButtonSize });
                        BeatSaberUI::SetButtonSprites(deleteButton, UITools::Base64ToSprite(trash_inactive), UITools::Base64ToSprite(trash));
                        BeatSaberUI::SetButtonSprites(copyButton, UITools::Base64ToSprite(copy_inactive), UITools::Base64ToSprite(copy));
                        BeatSaberUI::SetButtonSprites(saveButton, UITools::Base64ToSprite(save_inactive), UITools::Base64ToSprite(save));
                    //<modal id='load-dialog' hide-event='load-dialog-hide' move-to-center="true" click-off-closes="true" size-delta-x="70" size-delta-y="70">
                    //  <vertical spacing="10" pad="5">
                    //    <vertical spacing='5'>
                    //      <list id='load-file-list' expand-cell='true' select-cell='file-load-selected' list-style='Box' />
                    //    </vertical>
                    //    <horizontal preferred-width='60' bg='round-rect-panel' bg-color='#00000000'>
                    //      <page-button event-click='load-file-list#PageUp' direction='Up' />
                    //      <page-button event-click='load-file-list#PageDown' direction='Down' />
                    //    </horizontal>
                    //    <button text='Close' click-event='load-dialog-hide'/>
                    //  </vertical>
                    //</modal>
                    }

                    //<modal-color-picker id="color-picker-modal" value="brush-color-value" on-done="picker-selected-color" move-to-center="true" click-off-closes="true"></modal-color-picker>
                    brushList = BeatSaberUI::CreateScrollableCustomSourceList<CustomBrushListDataSource*>(horizontal->get_transform(), {35.0f, 60.0f}, [&](int idx){
                        SelectBrush(idx);
                    });

                    ReloadBrushList();

                    BrushTextures::LoadAllTextures();
                    textureList = BeatSaberUI::CreateScrollableList(horizontal->get_transform(), {22.0f, 60.0f}, [&](int idx){
                        SelectTexture(idx);
                    });

                    textureList->set_listStyle(CustomListTableData::ListStyle::Box);
                    textureList->cellSize = 22.0f;
                    ReloadTextureList();

                    effectList = BeatSaberUI::CreateScrollableList(horizontal->get_transform(), {35.0f, 60.0f}, [&](int idx){
                        SelectEffect(idx);
                    });
                    
                    effectList->set_listStyle(CustomListTableData::ListStyle::Simple);
                    effectList->cellSize = 5.5f;

                    ReloadEffectList();
                }
                //</horizontal>
                //<vertical child-expand-height="false" child-control-height="false" spacing="2" pad-left='40' pad-right='20'>
                {
                    auto bottomVertical = BeatSaberUI::CreateVerticalLayoutGroup(vertical->get_transform());
                    bottomVertical->set_childForceExpandHeight(false);
                    bottomVertical->set_childControlHeight(false);
                    bottomVertical->set_spacing(2);
                    bottomVertical->set_padding(UnityEngine::RectOffset::New_ctor(40, 20, 0, 0));

                    auto brush = GlobalBrushManager::get_activeBrush();
                    if (brush) size = brush->currentBrush.size;
                    //    <slider-setting id='SizeSlider' text='Brush Size' min='1' max='70' increment='1' integer-only='false' apply-on-change='true' value='Size' />
                    sizeSlider = BeatSaberUI::CreateSliderSetting(bottomVertical->get_transform(), "Brush Size", 1.0f, size, 1.0f, 70.0f, 0.5f, [&](float val){
                        size = (int)val;
                        auto brush = GlobalBrushManager::get_activeBrush();
                        if (brush) brush->currentBrush.size = size;
                    });
                    

                    //    <slider-setting id='GlowSlider' text='Glow Amount' min='0' max='1' increment='0.05' integer-only='false' apply-on-change='true' value='Glow' />
                    if (brush) glow = brush->currentBrush.glow;
                    glowSlider = BeatSaberUI::CreateSliderSetting(bottomVertical->get_transform(), "Glow Amount", 0.05f, glow, 0.0f, 1.0f, 0.5f, [&](float val){
                        glow = val;
                        auto brush = GlobalBrushManager::get_activeBrush();
                        if (brush) brush->currentBrush.glow = glow;
                    });

                    //    <slider-setting id='GlowSlider' text='Glow Amount' min='0' max='1' increment='0.05' integer-only='false' apply-on-change='true' value='Glow' />
                    if (brush) tile = brush->currentBrush.tiling.x;
                    tileSlider = BeatSaberUI::CreateSliderSetting(bottomVertical->get_transform(), "Tiling", 1.0f, tile, 1.0f, 20.0f, 0.5f, [&](float val){
                        tile = val;
                        auto brush = GlobalBrushManager::get_activeBrush();
                        if (brush) brush->currentBrush.tiling.x = tile;
                    });
                }
                //</vertical>
            }
            //</vertical>
            
            SetModalPosition(saveModal);
            SetModalPosition(loadModal);
            SetModalPosition(colorPickerModal->modalView);
            SetModalPosition(brushModal);

            ActiveControllerChanged(GlobalBrushManager::get_activeBrush());
        }
        
        GlobalBrushManager::OnActiveBrushChanged() += {&ScribbleViewController::ActiveControllerChanged, this};
    }

    void ScribbleViewController::SetModalPosition(HMUI::ModalView* modal)
    {
        modal->get_transform()->set_localPosition(Vector3(0, 0, -20));
        modal->get_transform()->set_localRotation(Quaternion::Euler(-20, 0, 0));
    }

    void ScribbleViewController::DidDeactivate(bool, bool)
    {
        GlobalBrushManager::OnActiveBrushChanged() -= {&ScribbleViewController::ActiveControllerChanged, this};
    }

    void ScribbleViewController::ReloadTextureList()
    {
        INFO("ReloadTextureList");
        textureList->data.clear();
        int row = reinterpret_cast<QuestUI::TableView*>(textureList->tableView)->get_selectedRow();
        auto textures = BrushTextures::GetTextures();
        for (auto& t : textures)
        {
            auto texture = t.second;
            if (texture)
            {
                auto sprite = Sprite::Create(texture, UnityEngine::Rect(0.0f, 0.0f, (float)texture->get_width(), (float)texture->get_height()), UnityEngine::Vector2(0.5f,0.5f), 1024.0f, 1u, UnityEngine::SpriteMeshType::FullRect, UnityEngine::Vector4(0.0f, 0.0f, 0.0f, 0.0f), false);
                Object::DontDestroyOnLoad(sprite);
                textureList->data.emplace_back(t.first, sprite);
            }
            else
                textureList->data.emplace_back(t.first);
        }

        textureList->tableView->ReloadData();
        textureList->tableView->RefreshCells(true, true);
        textureList->tableView->ScrollToCellWithIdx(row, HMUI::TableView::ScrollPositionType::Beginning, false);
    }

    void ScribbleViewController::ReloadEffectList()
    {
        INFO("ReloadEffectList");
        effectList->data.clear();
        int row = reinterpret_cast<QuestUI::TableView*>(effectList->tableView)->get_selectedRow();
        auto& effects = Effects::GetEffects();
        for (auto& e : effects)
        {
            effectList->data.emplace_back(e->get_name());
        }
        effectList->tableView->ReloadData();
        effectList->tableView->RefreshCells(true, true);
        effectList->tableView->ScrollToCellWithIdx(row, HMUI::TableView::ScrollPositionType::Beginning, false);
    }

    void ScribbleViewController::ReloadBrushList()
    {
        INFO("ReloadBrushList");
        brushList->data.clear();
        saveBrushList->data.clear();
        int firstRow = reinterpret_cast<QuestUI::TableView*>(brushList->tableView)->get_selectedRow();
        int secondRow = reinterpret_cast<QuestUI::TableView*>(saveBrushList->tableView)->get_selectedRow();

        for (auto b : Brushes::brushes)
        {
            auto texture = BrushTextures::GetTexture(b.textureName);
            if (texture)
            {
                auto sprite = Sprite::Create(texture, UnityEngine::Rect(0.0f, 0.0f, (float)texture->get_width(), (float)texture->get_height()), UnityEngine::Vector2(0.5f,0.5f), 1024.0f, 1u, UnityEngine::SpriteMeshType::FullRect, UnityEngine::Vector4(0.0f, 0.0f, 0.0f, 0.0f), false);
                Object::DontDestroyOnLoad(sprite);
                brushList->data.emplace_back(b.name, b.effectName, b.color, sprite);
                saveBrushList->data.emplace_back(b.name, b.effectName, b.color, sprite);
            }
            else
            {
                brushList->data.emplace_back(b.name, b.effectName, b.color);
                saveBrushList->data.emplace_back(b.name, b.effectName, b.color);
            }
        }

        brushList->tableView->ReloadData();
        brushList->tableView->RefreshCells(true, true);
        brushList->tableView->ScrollToCellWithIdx(firstRow > 0 ? firstRow : 0, HMUI::TableView::ScrollPositionType::Beginning, true);

        saveBrushList->tableView->ReloadData();
        saveBrushList->tableView->RefreshCells(true, true);
        saveBrushList->tableView->ClearSelection();
        saveBrushList->tableView->ScrollToCellWithIdx(secondRow > 0 ? secondRow : 0, HMUI::TableView::ScrollPositionType::Beginning, true);
    }
    
    void ScribbleViewController::SelectForBrush(const CustomBrush& brush)
    {
        int idx = 0;
        for (auto t : textureList->data)
        {
            if (t.text == brush.textureName) break; 
            idx++;
        }

        textureList->tableView->ScrollToCellWithIdx(idx, HMUI::TableView::ScrollPositionType::Beginning, true);
        textureList->tableView->SelectCellWithIdx(idx, false);

        idx = 0;
        for (auto t : effectList->data)
        {
            if (t.text == brush.effectName) break; 
            idx++;
        }

        effectList->tableView->ScrollToCellWithIdx(idx, HMUI::TableView::ScrollPositionType::Beginning, true);
        effectList->tableView->SelectCellWithIdx(idx, false);

        glowSlider->set_value(brush.glow);
        sizeSlider->set_value(brush.size);
        tileSlider->set_value(brush.tiling.x);
        colorPickerModal->set_color(brush.color);
        pickerImage->set_color(brush.color);
    }

    void ScribbleViewController::SelectBrush(int idx)
    {
        auto brush = GlobalBrushManager::get_activeBrush();
        if (brush)
        {
            auto& newBrush = Brushes::brushes[idx];
            brush->currentBrush = newBrush;
            
            SelectForBrush(newBrush);
        }
    }

    void ScribbleViewController::ActiveControllerChanged(BrushBehaviour* newBrush)
    {
        SelectForBrush(newBrush->currentBrush);

        int idx = 0;
        for (auto t : brushList->data)
        {
            if (t.text == newBrush->currentBrush.name) break; 
            idx++;
        }

        brushList->tableView->ScrollToCellWithIdx(idx, HMUI::TableView::ScrollPositionType::Beginning, true);
        brushList->tableView->SelectCellWithIdx(idx, false);

        eraserImage->set_color(newBrush->eraseMode ? Color(1.0, 0.0, 0.0, 1.0) : Color(1.0, 1.0, 1.0, 1.0));
        bucketImage->set_color(newBrush->bucketMode ? Color(0.0, 0.0, 1.0, 1.0) : Color(1.0, 1.0, 1.0, 1.0));
    }

    void ScribbleViewController::SelectTexture(int idx)
    {
        auto brush = GlobalBrushManager::get_activeBrush();
        if (brush) 
        {
            // update current brush
            std::string texName = BrushTextures::GetTextureName(idx);
            auto& currentBrush = brush->currentBrush;
            currentBrush.textureName = texName;
        }
    }

    void ScribbleViewController::SelectEffect(int idx)
    {
        auto brush = GlobalBrushManager::get_activeBrush();
        if (brush) 
        {
            auto& currentBrush = brush->currentBrush;
            std::string effectName = Effects::GetEffectName(idx);
            currentBrush.effectName = effectName;
        }
    }

    void ScribbleViewController::SelectPicker()
    {
        if (colorPickerModal) colorPickerModal->Show();
    }

    void ScribbleViewController::PickerSelectedColor(Color color)
    {
        auto brush = GlobalBrushManager::get_activeBrush();
        if (brush)
        {
            brush->currentBrush.color = color;
            pickerImage->set_color(color);
        }
    }
    void ScribbleViewController::SelectBucketMode()
    {
        auto brush = GlobalBrushManager::get_activeBrush();
        if (brush)
        {
            brush->bucketMode ^= 1;
            bucketImage->set_color(brush->bucketMode ? Color(0.0, 0.0, 1.0, 1.0) : Color(1.0, 1.0, 1.0, 1.0));
        }
    }

    void ScribbleViewController::SelectEraseMode()
    {
        auto brush = GlobalBrushManager::get_activeBrush();
        if (brush)
        {
            brush->eraseMode ^= 1;
            eraserImage->set_color(brush->eraseMode ? Color(1.0, 0.0, 0.0, 1.0) : Color(1.0, 1.0, 1.0, 1.0));
        }
    }

    void ScribbleViewController::ShowSaveFile()
    {
        if (saveModal) saveModal->Show(true, true, nullptr);
    }

    void ScribbleViewController::ShowLoadFile()
    {
        if (loadModal) loadModal->Show(true, true, nullptr);
    }

    void ScribbleViewController::SaveFilenameChanged(std::string val)
    {
        saveFileName = val;
    }

    void ScribbleViewController::BrushNameChanged(std::string val)
    {
        brushName = val;
    }

    void ScribbleViewController::ReloadFileLists()
    {
        std::vector<std::string> fileNames = {};
        FileUtils::GetFilesInFolderPath("png", drawingPath, fileNames);

        for (auto& f : fileNames)
        {
            std::string name = FileUtils::GetFileName(f, true);
            
            bool addSave = std::find_if(saveFileList->data.begin(), saveFileList->data.end(), [&](const auto& x) { return x.text == name; }) == saveFileList->data.end();
            bool addLoad = std::find_if(loadFileList->data.begin(), loadFileList->data.end(), [&](const auto& x) { return x.text == name; }) == loadFileList->data.end();
            
            if (addSave || addLoad)
            {
                std::ifstream reader(string_format("%s/%s", drawingPath, f.c_str()), std::ios::in | std::ios::binary);
                long size;
                UnityEngine::Texture2D* texture = nullptr;
                
                if (ThumbnailHelper::CheckPngData(reader, size, false)) // if success
                {
                    INFO("Reading texture from ifstream");
                    texture = ThumbnailHelper::ReadPNG(reader, size);
                }

                auto sprite = texture ? Sprite::Create(texture, UnityEngine::Rect(0.0f, 0.0f, (float)texture->get_width(), (float)texture->get_height()), UnityEngine::Vector2(0.5f,0.5f), 1024.0f, 1u, UnityEngine::SpriteMeshType::FullRect, UnityEngine::Vector4(0.0f, 0.0f, 0.0f, 0.0f), false) : nullptr;
                if (addSave)
                    saveFileList->data.emplace_back(name, sprite);
                if (addLoad)
                    loadFileList->data.emplace_back(name, sprite);
            }
        }

        saveFileList->tableView->ReloadData();
        loadFileList->tableView->ReloadData();

        saveFileList->tableView->RefreshCells(true, true);
        loadFileList->tableView->RefreshCells(true, true);
    }

    void ScribbleViewController::SaveSelectIdx(int idx)
    {
        fileNameField->set_text(il2cpp_utils::newcsstr(saveFileList->data[idx].text));
    }
}