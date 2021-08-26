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

#include "System/Collections/Generic/HashSet_1.hpp"

#include "Utils/FileUtils.hpp"
#include "Utils/ThumbnailHelper.hpp"

DEFINE_TYPE(Scribble, ScribbleViewController);

using namespace QuestUI;
using namespace UnityEngine;
using namespace UnityEngine::UI;

namespace Scribble
{
    void ScribbleViewController::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling)
    {
        if (firstActivation)
        {
            // going to attempt to port the UI from BSML
            
            //<vertical anchor-pos-y="-25" child-expand-height="false" child-control-height="false" spacing="2" xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance' xsi:schemaLocation='https://monkeymanboy.github.io/BSML-Docs/ https://raw.githubusercontent.com/monkeymanboy/BSML-Docs/gh-pages/BSMLSchema.xsd'>
            {
            auto vertical = BeatSaberUI::CreateVerticalLayoutGroup(get_transform());
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
                            layoutButtons->set_preferredWidth(12);
                            eraserPickerVertical->set_spacing(2);
                            eraserPickerVertical->set_childControlHeight(true);
                            eraserPickerVertical->set_childControlWidth(true);
                            eraserPickerVertical->set_childForceExpandHeight(false);
                            eraserPickerVertical->set_childForceExpandWidth(false);

                            auto pickerButton = BeatSaberUI::CreateUIButton(eraserPickerVertical->get_transform(), "", "PracticeButton", Vector2(12, 12), std::bind(&ScribbleViewController::SelectPicker, this));
                            
                            //<button-with-icon id="picker-btn" stroke-type="Clean" preferred-width="12" preferred-height="12" on-click="selectPicker" click-event="show-picker" hover-hint="Color Picker" />
                            auto eraserButton = BeatSaberUI::CreateUIButton(eraserPickerVertical->get_transform(), "", "PracticeButton", Vector2(12, 12), std::bind(&ScribbleViewController::SelectEraseMode, this));

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
                    //  <vertical spacing="8" pad="5">
                    auto saveHorizontal = BeatSaberUI::CreateHorizontalLayoutGroup(saveModal->get_transform());
                    saveHorizontal->set_childForceExpandWidth(false);
                    saveHorizontal->set_childForceExpandHeight(true);

                    saveFileList = BeatSaberUI::CreateScrollableList(saveHorizontal->get_transform(), Vector2(30, 76));
                    saveFileList->set_listStyle(QuestUI::CustomListTableData::ListStyle::Box);
                    saveFileList->cellSize = 30.0f;
                    //    </vertical>

                    //    <vertical  bg='round-rect-panel' bg-color='#00000000' preferred-height='180'>
                    auto saveInfoVertical = BeatSaberUI::CreateVerticalLayoutGroup(saveHorizontal->get_transform());
                    saveInfoVertical->get_gameObject()->AddComponent<QuestUI::Backgroundable*>()->ApplyBackgroundWithAlpha(il2cpp_utils::newcsstr("round-rect-panel"), 0.2f);
                    saveInfoVertical->set_padding(UnityEngine::RectOffset::New_ctor(0, 0, 20, 20));
                    saveInfoVertical->get_gameObject()->AddComponent<LayoutElement*>()->set_preferredHeight(40);
                    saveInfoVertical->set_childForceExpandHeight(false);
                    //      <horizontal preferred-width='60'>
                    // we dont need no scroll buttons cause we use a scrollable list
                    //        <page-button event-click='save-file-list#PageUp' direction='Up' />
                    //        <page-button event-click='save-file-list#PageDown' direction='Down' />
                    //      </horizontal>
                    //      <string-setting id='new-file-string' text='Name' value='save-file-name' on-change='save-file-name-changed' preferred-width='10' apply-on-change='true'></string-setting>
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
                    //      <horizontal>
                    //        <button text='Close' click-event='save-dialog-hide'/>
                    //        <button text='New' on-click='save-new'/>
                    //      </horizontal>
                    //    </vertical>
                    //  </vertical>
                    //</modal>

                    loadModal = BeatSaberUI::CreateModal(horizontal->get_transform(), Vector2(80, 80), nullptr, true);
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

                    auto loadConfirmHorizontal = BeatSaberUI::CreateHorizontalLayoutGroup(loadInfoVertical->get_transform());
                    auto loadClose = BeatSaberUI::CreateUIButton(loadConfirmHorizontal->get_transform(), "Close", [&](){ loadModal->Hide(true, nullptr); });
                    auto loadLoad = BeatSaberUI::CreateUIButton(loadConfirmHorizontal->get_transform(), "Load", [&](){ 
                        if (reinterpret_cast<System::Collections::ICollection*>(loadFileList->tableView->selectedCellIdxs)->get_Count() <= 0) return;
                        auto enumerator = loadFileList->tableView->selectedCellIdxs->GetEnumerator();
                        INFO("Initial value: %d", enumerator.current);
                        if (!enumerator.MoveNext()) return;
                        //loadFileList->tableView->selectedCellIdxs->items->values[0];
                        ScribbleContainer::get_instance()->Load(string_format("%s/%s.png", drawingPath, loadFileList->data[enumerator.current].text.c_str()));
                    });

                    ReloadFileLists();
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

                    //<modal-color-picker id="color-picker-modal" value="brush-color-value" on-done="picker-selected-color" move-to-center="true" click-off-closes="true"></modal-color-picker>
                    brushList = BeatSaberUI::CreateScrollableList(horizontal->get_transform(), {35.0f, 60.0f}, [&](int idx){
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
                    sizeSlider = BeatSaberUI::CreateSliderSetting(bottomVertical->get_transform(), "Brush Size", 1.0f, size, 1.0f, 70.0f, [&](float val){
                        size = (int)val;
                        auto brush = GlobalBrushManager::get_activeBrush();
                        if (brush) brush->currentBrush.size = size;
                    });
                    

                    //    <slider-setting id='GlowSlider' text='Glow Amount' min='0' max='1' increment='0.05' integer-only='false' apply-on-change='true' value='Glow' />
                    if (brush) glow = brush->currentBrush.glow;
                    glowSlider = BeatSaberUI::CreateSliderSetting(bottomVertical->get_transform(), "Glow Amount", 0.05f, glow, 0.0f, 1.0f, [&](float val){
                        glow = val;
                        auto brush = GlobalBrushManager::get_activeBrush();
                        if (brush) brush->currentBrush.glow = glow;
                    });

                    //    <slider-setting id='GlowSlider' text='Glow Amount' min='0' max='1' increment='0.05' integer-only='false' apply-on-change='true' value='Glow' />
                    if (brush) tile = brush->currentBrush.tiling.x;
                    tileSlider = BeatSaberUI::CreateSliderSetting(bottomVertical->get_transform(), "Tiling", 0.1f, tile, 0.0f, 20.0f, [&](float val){
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

            SelectForBrush(GlobalBrushManager::get_activeBrush()->currentBrush);
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
        int row = brushList->tableView->get_contentTransform() ? reinterpret_cast<QuestUI::TableView*>(brushList->tableView)->get_scrolledRow() : 0;
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
        textureList->tableView->ScrollToCellWithIdx(row, HMUI::TableView::ScrollPositionType::Beginning, false);
    }

    void ScribbleViewController::ReloadEffectList()
    {
        INFO("ReloadEffectList");
        effectList->data.clear();
        int row = brushList->tableView->get_contentTransform() ? reinterpret_cast<QuestUI::TableView*>(brushList->tableView)->get_scrolledRow() : 0;
        auto& effects = Effects::GetEffects();
        for (auto& e : effects)
        {
            effectList->data.emplace_back(e->get_name());
        }
        effectList->tableView->ReloadData();
        effectList->tableView->ScrollToCellWithIdx(row, HMUI::TableView::ScrollPositionType::Beginning, false);
    }

    void ScribbleViewController::ReloadBrushList()
    {
        INFO("ReloadBrushList");
        brushList->data.clear();
        INFO("cleared");
        int row = brushList->tableView->get_contentTransform() ? reinterpret_cast<QuestUI::TableView*>(brushList->tableView)->get_scrolledRow() : 0;
        INFO("row");

        for (auto b : Brushes::brushes)
        {
            INFO("brush %s", b.name.c_str());
            auto texture = BrushTextures::GetTexture(b.textureName);
            if (texture)
            {
                auto sprite = Sprite::Create(texture, UnityEngine::Rect(0.0f, 0.0f, (float)texture->get_width(), (float)texture->get_height()), UnityEngine::Vector2(0.5f,0.5f), 1024.0f, 1u, UnityEngine::SpriteMeshType::FullRect, UnityEngine::Vector4(0.0f, 0.0f, 0.0f, 0.0f), false);
                Object::DontDestroyOnLoad(sprite);
                brushList->data.emplace_back(b.name, b.effectName, sprite);
            }
            else
                brushList->data.emplace_back(b.name, b.effectName);
        }

        INFO("reload");
        brushList->tableView->ReloadData();
        INFO("scroll");
        brushList->tableView->ScrollToCellWithIdx(row, HMUI::TableView::ScrollPositionType::Beginning, false);
        INFO("end");
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
    }

    void ScribbleViewController::SaveSelectIdx(int idx)
    {
        fileNameField->set_text(il2cpp_utils::newcsstr(saveFileList->data[idx].text));
    }
}