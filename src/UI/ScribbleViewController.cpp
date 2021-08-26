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
                            //BeatSaberUI::SetButtonSprites(pickerButton, sprite, sprite);

                            std::string eraserPath = "/sdcard/ModData/com.beatgames.beatsaber/Mods/Scribble/eraser.png";
                            sprite = BeatSaberUI::FileToSprite(eraserPath);
                            eraserImage = UITools::CreateImage(eraserButton->get_transform(), {0, 0}, {12, 12});
                            eraserImage->set_material(material);
                            eraserImage->set_sprite(sprite);
                            //BeatSaberUI::SetButtonSprites(eraserButton, sprite, sprite);

                            //reinterpret_cast<RectTransform*>(pickerButton->get_transform()->GetChild(0))->set_sizeDelta({12, 12});
                            //reinterpret_cast<RectTransform*>(eraserButton->get_transform()->GetChild(0))->set_sizeDelta({12, 12});

                            //<button-with-icon id="eraser-btn" stroke-type="Clean" preferred-width="12" preferred-height="12" on-click="selectEraseMode" hover-hint="Eraser" />
                        }
                        //  </vertical>
                    }
                    //</vertical>

                    UnityEngine::Color color = {0, 0, 0, 1.0};
                    if (GlobalBrushManager::get_activeBrush()) color = GlobalBrushManager::get_activeBrush()->currentBrush.color;
                    colorPickerModal = BeatSaberUI::CreateColorPickerModal(get_transform(), "", color, std::bind(&ScribbleViewController::PickerSelectedColor, this, std::placeholders::_1));
                    
                    //<modal id='save-dialog' hide-event='save-dialog-hide' move-to-center="true" click-off-closes="true" size-delta-x="70" size-delta-y="80">
                    saveModal = BeatSaberUI::CreateModal(horizontal->get_transform(), Vector2(70, 80), nullptr, true);
                    //  <vertical spacing="8" pad="5">
                    auto saveVertical = BeatSaberUI::CreateVerticalLayoutGroup(saveModal->get_transform());
                    saveVertical->set_padding(UnityEngine::RectOffset::New_ctor(5, 5, 5, 5));
                    saveVertical->set_spacing(8);

                    //    <vertical spacing='5'>
                    auto saveFileVertical = BeatSaberUI::CreateVerticalLayoutGroup(saveVertical->get_transform());
                    saveFileVertical->set_spacing(5);
                    //      <list id='save-file-list' expand-cell='true' select-cell='file-save-selected' list-style='Box' />
                    saveFileList = BeatSaberUI::CreateScrollableList(saveFileVertical->get_transform(), Vector2(35, 60));
                    saveFileList->set_listStyle(QuestUI::CustomListTableData::ListStyle::Simple);
                    saveFileList->cellSize = 5.5f;
                    //    </vertical>

                    //    <vertical  bg='round-rect-panel' bg-color='#00000000' preferred-height='180'>
                    auto infoVertical = BeatSaberUI::CreateVerticalLayoutGroup(saveVertical->get_transform());
                    infoVertical->get_gameObject()->AddComponent<QuestUI::BackGroundable*>()->ApplyBackgroundWithAlpha(il2cpp_utils::newcsstr("round-rect-panel"), 0.2f); 
                    infoVertical->get_gameObject()->AddComponent<LayoutElement*>()->set_preferredHeight(180);
                    //      <horizontal preferred-width='60'>

                    //        <page-button event-click='save-file-list#PageUp' direction='Up' />
                    //        <page-button event-click='save-file-list#PageDown' direction='Down' />
                    //      </horizontal>
                    
                    //      <string-setting id='new-file-string' text='Name' value='save-file-name' on-change='save-file-name-changed' preferred-width='10' apply-on-change='true'></string-setting>
                    //      <horizontal>
                    //        <button text='Close' click-event='save-dialog-hide'/>
                    //        <button text='New' on-click='save-new'/>
                    //      </horizontal>
                    //    </vertical>
                    //  </vertical>
                    //</modal>

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
                }
                //</vertical>
            }
            //</vertical>
        }
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

    void ScribbleViewController::SelectBrush(int idx)
    {
        auto brush = GlobalBrushManager::get_activeBrush();
        if (brush) 
        {
            auto& newBrush = Brushes::brushes[idx];
            brush->currentBrush = newBrush;
            glowSlider->set_value(newBrush.glow);
            sizeSlider->set_value(newBrush.size);
            colorPickerModal->set_color(newBrush.color);
        }
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

            // update Brush list object
            //auto foundBrush = Brushes::GetBrush(currentBrush.name);
            //if (foundBrush)
            //    (*foundBrush).get().textureName = texName;

            //Brushes::Save();
            //ReloadBrushList();
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

            //auto foundBrush = Brushes::GetBrush(currentBrush.name);
            //if (foundBrush)
            //    (*foundBrush).get().effectName = effectName;

            //Brushes::Save();
            //ReloadBrushList();
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
}