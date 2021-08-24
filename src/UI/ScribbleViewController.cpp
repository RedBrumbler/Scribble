#include "UI/ScribbleViewController.hpp"
#include "UI/UITools.hpp"
#include "ScribbleContainer.hpp"
#include "Brushes.hpp"
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

#include "questui/shared/BeatSaberUI.hpp"

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

                            auto pickerButton = BeatSaberUI::CreateUIButton(eraserPickerVertical->get_transform(), "", "PracticeButton", Vector2(12, 12), [&](){
                                colorPickerModal->Show(true, true, nullptr);
                            });
                            
                            //<button-with-icon id="picker-btn" stroke-type="Clean" preferred-width="12" preferred-height="12" on-click="selectPicker" click-event="show-picker" hover-hint="Color Picker" />
                            auto eraserButton = BeatSaberUI::CreateUIButton(eraserPickerVertical->get_transform(), "", "PracticeButton", Vector2(12, 12), [&](){
                                SelectEraseMode();
                            });

                            std::string pickerPath = "/sdcard/ModData/com.beatgames.beatsaber/Mods/Scribble/picker.png";
                            auto sprite = BeatSaberUI::FileToSprite(pickerPath);
                            auto img = UITools::CreateImage(pickerButton->get_transform(), {0, 0}, {12, 12});
                            img->set_sprite(sprite);
                            //BeatSaberUI::SetButtonSprites(pickerButton, sprite, sprite);

                            std::string eraserPath = "/sdcard/ModData/com.beatgames.beatsaber/Mods/Scribble/eraser.png";
                            sprite = BeatSaberUI::FileToSprite(eraserPath);
                            img = UITools::CreateImage(eraserButton->get_transform(), {0, 0}, {12, 12});
                            img->set_sprite(sprite);
                            //BeatSaberUI::SetButtonSprites(eraserButton, sprite, sprite);

                            //reinterpret_cast<RectTransform*>(pickerButton->get_transform()->GetChild(0))->set_sizeDelta({12, 12});
                            //reinterpret_cast<RectTransform*>(eraserButton->get_transform()->GetChild(0))->set_sizeDelta({12, 12});

                            //<button-with-icon id="eraser-btn" stroke-type="Clean" preferred-width="12" preferred-height="12" on-click="selectEraseMode" hover-hint="Eraser" />
                        }
                        //  </vertical>
                    }
                    //</vertical>

                    colorPickerModal = BeatSaberUI::CreateModal(get_transform(), Vector2(80.0f, 40.0f), nullptr);
                    UnityEngine::Color color = {0, 0, 0, 1.0};
                    if (GlobalBrushManager::get_activeBrush()) color = GlobalBrushManager::get_activeBrush()->currentBrush.color;
                    BeatSaberUI::CreateColorPicker(colorPickerModal->get_transform(), "Brush Color", color, [&](UnityEngine::Color col, GlobalNamespace::ColorChangeUIEventType eventType) {
                        PickerSelectedColor(col);
                    });

                    //<modal-color-picker id="color-picker-modal" value="brush-color-value" on-done="picker-selected-color" move-to-center="true" click-off-closes="true"></modal-color-picker>
                    auto brushList = BeatSaberUI::CreateScrollableList(horizontal->get_transform(), {35.0f, 60.0f}, [&](int idx){
                        SelectBrush(idx);
                    });

                    for (auto b : Brushes::brushes)
                    {
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

                    brushList->tableView->ReloadData();
                
                    auto textureList = BeatSaberUI::CreateScrollableList(horizontal->get_transform(), {35.0f, 60.0f}, [&](int idx){
                        SelectTexture(idx);
                    });

                    auto effectsList = BeatSaberUI::CreateScrollableList(horizontal->get_transform(), {35.0f, 60.0f}, [&](int idx){
                        SelectEffect(idx);
                    });
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
                    BeatSaberUI::CreateIncrementSetting(bottomVertical->get_transform(), "Brush Size", 0, 1.0f, size, 1.0f, 70.0f, [&](float val){
                        size = (int)val;
                        auto brush = GlobalBrushManager::get_activeBrush();
                        if (brush) brush->currentBrush.size = size;
                    });

                    //    <slider-setting id='GlowSlider' text='Glow Amount' min='0' max='1' increment='0.05' integer-only='false' apply-on-change='true' value='Glow' />
                    if (brush) glow = brush->currentBrush.glow;
                    BeatSaberUI::CreateIncrementSetting(bottomVertical->get_transform(), "Glow Amount", 2, 0.05f, size, 0.0f, 1.0f, [&](float val){
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

    void ScribbleViewController::SelectBrush(int idx)
    {

    }

    void ScribbleViewController::SelectTexture(int idx)
    {

    }

    void ScribbleViewController::SelectEffect(int idx)
    {

    }

    void ScribbleViewController::SelectPicker()
    {
        if (colorPickerModal) colorPickerModal->Show(true, true, nullptr);
    }

    void ScribbleViewController::PickerSelectedColor(Color color)
    {

    }

    void ScribbleViewController::SelectEraseMode()
    {

    }
}