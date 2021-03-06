#include "UI/ScribbleViewController.hpp"
#include "UI/UITools.hpp"
#include "config.hpp"

#include "BrushTextures.hpp"
#include "Brushes.hpp"
#include "Effects.hpp"
#include "GlobalBrushManager.hpp"
#include "ScribbleContainer.hpp"
#include "icons.hpp"
#include "logging.hpp"
#include "static-defines.hpp"

#include "UnityEngine/Rect.hpp"
#include "UnityEngine/RectOffset.hpp"
#include "UnityEngine/Sprite.hpp"
#include "UnityEngine/SpriteMeshType.hpp"
#include "UnityEngine/UI/ContentSizeFitter.hpp"
#include "UnityEngine/Vector2.hpp"
#include "UnityEngine/Vector4.hpp"

#include "GlobalNamespace/BoolSO.hpp"
#include "GlobalNamespace/MainEffectContainerSO.hpp"

#include "questui/shared/BeatSaberUI.hpp"
#include "questui/shared/CustomTypes/Components/Backgroundable.hpp"
#include "questui/shared/CustomTypes/Components/List/QuestUITableView.hpp"

#include "System/Collections/Generic/HashSet_1.hpp"

#include "HMUI/TableView_ScrollPositionType.hpp"

#include "Utils/FileUtils.hpp"
#include "Utils/ThumbnailHelper.hpp"

DEFINE_TYPE(Scribble, ScribbleViewController);

using namespace QuestUI;
using namespace UnityEngine;
using namespace UnityEngine::UI;

namespace Scribble
{
    std::string ScribbleViewController::FindNextName(std::string name, int idx)
    {
        INFO("Getting candidate for %s", name.data());
        // candidate, what we think might work
        int foundDash = name.find('-');
        std::string candidate;
        if (foundDash != std::string::npos)
        {
            INFO("Found a -");
            candidate = idx == 0 ? std::string(name.substr(0, foundDash)) : string_format("%s-%d", std::string(name.substr(0, foundDash)).c_str(), idx);
        }
        else
        {
            INFO("Didnt find a -");
            candidate = idx == 0 ? std::string(name) : string_format("%s-%d", name.data(), idx);
        }
        INFO("Candidate %s", candidate.c_str());
        // check all existing brushes for this name
        for (auto b : Brushes::brushes)
        {
            // if we find the name, increment idx by 1 and try again
            if (b.name == candidate)
                return FindNextName(name, ++idx);
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
            auto vertical = BeatSaberUI::CreateVerticalLayoutGroup(get_transform());
            vertical->get_gameObject()->GetComponent<Backgroundable*>()->ApplyBackgroundWithAlpha("round-rect-panel", 0.5f);
            vertical->get_rectTransform()->set_anchoredPosition({0.0f, -25.0f});
            vertical->set_childForceExpandHeight(false);
            vertical->set_childControlHeight(false);
            vertical->set_spacing(2);
            vertical->set_padding(RectOffset::New_ctor(5, 5, 0, 0));

            //<horizontal horizontal-fit="PreferredSize" child-expand-width="false" child-expand-height="true" child-control-width="true" child-control-height="true" spacing="5">

            auto horizontal = BeatSaberUI::CreateHorizontalLayoutGroup(vertical->get_transform());
            auto sizeFitter = horizontal->get_gameObject()->AddComponent<ContentSizeFitter*>();
            sizeFitter->set_horizontalFit(ContentSizeFitter::FitMode::PreferredSize);
            horizontal->set_childForceExpandWidth(false);
            horizontal->set_childForceExpandHeight(true);
            horizontal->set_childControlWidth(true);
            horizontal->set_childControlHeight(true);
            horizontal->set_spacing(5);
            //<vertical pad-top="5">
            CreateLeftToolBar(horizontal->get_transform());

            //<modal id='save-dialog' hide-event='save-dialog-hide' move-to-center="true" click-off-closes="true" size-delta-x="70" size-delta-y="80">
            saveModal = BeatSaberUI::CreateModal(horizontal->get_transform(), Vector2(80, 80), nullptr, true);

            //  <vertical spacing="8" pad="5">
            auto saveHorizontal = BeatSaberUI::CreateHorizontalLayoutGroup(saveModal->get_transform());
            saveHorizontal->set_childForceExpandWidth(false);
            saveHorizontal->set_childForceExpandHeight(true);

            saveFileList = BeatSaberUI::CreateScrollableList(saveHorizontal->get_transform(), Vector2(30, 76), std::bind(&ScribbleViewController::SaveSelectIdx, this, std::placeholders::_1));
            saveFileList->set_listStyle(QuestUI::CustomListTableData::ListStyle::Box);
            saveFileList->cellSize = 30.0f;

            auto saveInfoVertical = BeatSaberUI::CreateVerticalLayoutGroup(saveHorizontal->get_transform());
            saveInfoVertical->get_gameObject()->AddComponent<QuestUI::Backgroundable*>()->ApplyBackgroundWithAlpha("round-rect-panel", 0.2f);
            saveInfoVertical->set_padding(UnityEngine::RectOffset::New_ctor(0, 0, 20, 20));
            saveInfoVertical->get_gameObject()->AddComponent<LayoutElement*>()->set_preferredHeight(40);
            saveInfoVertical->set_childForceExpandHeight(false);
            auto saveFileNameHorizontal = BeatSaberUI::CreateHorizontalLayoutGroup(saveInfoVertical->get_transform());
            saveFileNameHorizontal->set_childForceExpandHeight(false);

            fileNameField = BeatSaberUI::CreateStringSetting(saveFileNameHorizontal->get_transform(), u"filename", "", std::bind(&ScribbleViewController::SaveFilenameChanged, this, std::placeholders::_1));
            auto nameFieldLayout = fileNameField->GetComponent<LayoutElement*>();
            nameFieldLayout->set_preferredWidth(30);

            auto saveConfirmHorizontal = BeatSaberUI::CreateHorizontalLayoutGroup(saveInfoVertical->get_transform());
            saveConfirmHorizontal->set_childForceExpandHeight(false);
            auto saveClose = BeatSaberUI::CreateUIButton(saveConfirmHorizontal->get_transform(), "Close", [&]()
                                                         {
                                                             saveModal->Hide(true, nullptr);
                                                             saveFileName = "";
                                                             fileNameField->set_text("");
                                                             ReloadFileLists();
                                                         });
            auto saveNew = BeatSaberUI::CreateUIButton(saveConfirmHorizontal->get_transform(), "Save", [&]()
                                                       {
                                                           if (ScribbleContainer::get_instance()->get_IsInAnimation())
                                                               return;
                                                           ScribbleContainer::get_instance()->Save(string_format("%s/%s.png", drawingPath, saveFileName.c_str()));
                                                       });

            loadModal = BeatSaberUI::CreateModal(horizontal->get_transform(), Vector2(80, 80), nullptr, true);

            auto loadHorizontal = BeatSaberUI::CreateHorizontalLayoutGroup(loadModal->get_transform());
            loadHorizontal->set_childForceExpandWidth(false);
            loadHorizontal->set_childForceExpandHeight(true);

            loadFileList = BeatSaberUI::CreateScrollableList(loadHorizontal->get_transform(), Vector2(30, 76));
            loadFileList->set_listStyle(QuestUI::CustomListTableData::ListStyle::Box);
            loadFileList->cellSize = 30.0f;

            auto loadInfoVertical = BeatSaberUI::CreateVerticalLayoutGroup(loadHorizontal->get_transform());
            loadInfoVertical->get_gameObject()->AddComponent<QuestUI::Backgroundable*>()->ApplyBackgroundWithAlpha("round-rect-panel", 0.2f);
            loadInfoVertical->get_gameObject()->AddComponent<LayoutElement*>()->set_preferredHeight(40);
            loadInfoVertical->get_gameObject()->AddComponent<LayoutElement*>()->set_preferredWidth(40);
            loadInfoVertical->set_childForceExpandHeight(false);
            loadInfoVertical->set_padding(UnityEngine::RectOffset::New_ctor(0, 0, 20, 20));

            BeatSaberUI::CreateToggle(loadInfoVertical->get_transform(), "Animated", config.loadAnimated, [](bool value)
                                      { config.loadAnimated = value; });
            auto loadButtonHorizontal = BeatSaberUI::CreateHorizontalLayoutGroup(loadInfoVertical->get_transform());
            auto loadClose = BeatSaberUI::CreateUIButton(loadButtonHorizontal->get_transform(), "Close", [&]()
                                                         { loadModal->Hide(true, nullptr); });
            auto loadLoad = BeatSaberUI::CreateUIButton(loadButtonHorizontal->get_transform(), "Load", [&]()
                                                        {
                                                            int idx = reinterpret_cast<QuestUI::TableView*>(loadFileList->tableView)->get_selectedRow();
                                                            if (idx < 0)
                                                                return;
                                                            //loadFileList->tableView->selectedCellIdxs->items->values[0];
                                                            if (ScribbleContainer::get_instance()->get_IsInAnimation())
                                                                return;
                                                            ScribbleContainer::get_instance()->Load(string_format("%s/%s.png", drawingPath, loadFileList->data[idx].text.c_str()), true, config.loadAnimated);
                                                            loadFileList->tableView->ClearSelection();
                                                        });

            // model load modal
            modelsModal = BeatSaberUI::CreateModal(horizontal->get_transform(), Vector2(80, 80), nullptr, true);

            auto modelHorizontal = BeatSaberUI::CreateHorizontalLayoutGroup(modelsModal->get_transform());
            modelHorizontal->set_childForceExpandWidth(false);
            modelHorizontal->set_childForceExpandHeight(true);

            modelFileList = BeatSaberUI::CreateScrollableList(modelHorizontal->get_transform(), Vector2(30, 76));
            modelFileList->set_listStyle(QuestUI::CustomListTableData::ListStyle::Simple);
            //modelFileList->cellSize = 30.0f;

            auto modelInfoVertical = BeatSaberUI::CreateVerticalLayoutGroup(modelHorizontal->get_transform());
            modelInfoVertical->get_gameObject()->AddComponent<QuestUI::Backgroundable*>()->ApplyBackgroundWithAlpha("round-rect-panel", 0.2f);
            modelInfoVertical->get_gameObject()->AddComponent<LayoutElement*>()->set_preferredHeight(40);
            modelInfoVertical->get_gameObject()->AddComponent<LayoutElement*>()->set_preferredWidth(40);
            modelInfoVertical->set_childForceExpandHeight(false);
            modelInfoVertical->set_padding(UnityEngine::RectOffset::New_ctor(0, 0, 20, 20));

            BeatSaberUI::CreateToggle(modelInfoVertical->get_transform(), "Animated", config.modelAnimated, [](bool value)
                                      { config.modelAnimated = value; });
            auto modelButtonHorizontal = BeatSaberUI::CreateHorizontalLayoutGroup(modelInfoVertical->get_transform());
            auto modelClose = BeatSaberUI::CreateUIButton(modelButtonHorizontal->get_transform(), "Close", [&]()
                                                          { modelsModal->Hide(true, nullptr); });
            auto modelLoad = BeatSaberUI::CreateUIButton(modelButtonHorizontal->get_transform(), "Load", [&]()
                                                         {
                                                             int idx = reinterpret_cast<QuestUI::TableView*>(modelFileList->tableView)->get_selectedRow();
                                                             if (idx < 0)
                                                                 return;
                                                             //modelFileList->tableView->selectedCellIdxs->items->values[0];
                                                             if (ScribbleContainer::get_instance()->get_IsInAnimation())
                                                                 return;
                                                             ScribbleContainer::get_instance()->Load(string_format("%s/%s.obj", modelsPath, modelFileList->data[idx].text.c_str()), true, config.modelAnimated);
                                                             modelFileList->tableView->ClearSelection();
                                                         });

            ReloadFileLists();

            settingsModal = BeatSaberUI::CreateModal(
                horizontal->get_transform(), Vector2(50, 50), [](auto x)
                { SaveConfig(); },
                true);
            auto settingsVertical = BeatSaberUI::CreateScrollableModalContainer(settingsModal);
            auto toggle = BeatSaberUI::CreateToggle(settingsVertical->get_transform(), "Visible during play", config.visibleDuringPlay, [](bool val)
                                                    {
                                                        config.visibleDuringPlay = val;
                                                        SaveConfig();
                                                    });
            BeatSaberUI::AddHoverHint(toggle->get_gameObject(), "Whether or not your drawing is visible during gameplay");

            // if post processing enabled, allow to turn it off for scribble
            auto effectContainers = Resources::FindObjectsOfTypeAll<GlobalNamespace::MainEffectContainerSO*>();
            if (effectContainers && effectContainers->Length() > 0)
            {
                auto effectContainer = effectContainers->values[0];
                if (effectContainer->dyn__postProcessEnabled()->get_value())
                {
                    toggle = BeatSaberUI::CreateToggle(settingsVertical->get_transform(), "Use Real Glow", config.useRealGlow, [](bool val)
                                                       {
                                                           config.useRealGlow = val;
                                                           SaveConfig();
                                                           ScribbleContainer::SetRealGlow(val);
                                                       });
                    BeatSaberUI::AddHoverHint(toggle->get_gameObject(), "Whether the mod uses real glow or just makes lines more white when glow is high");
                }
            }
            CreateMainVertical(horizontal->get_transform());

            SetModalPosition(saveModal);
            SetModalPosition(loadModal);
            SetModalPosition(colorPickerModal->modalView);
            SetModalPosition(settingsModal);
            SetModalPosition(modelsModal);

            ActiveControllerChanged(GlobalBrushManager::get_activeBrush());
        }

        GlobalBrushManager::OnActiveBrushChanged() += {&ScribbleViewController::ActiveControllerChanged, this};
    }

    void ScribbleViewController::DeleteBrush()
    {
        int idx = reinterpret_cast<QuestUI::TableView*>(brushList->tableView)->get_selectedRow();
        int row = reinterpret_cast<QuestUI::TableView*>(brushList->tableView)->get_scrolledRow();
        INFO("brush delete idx: %d", idx);
        if (idx < 0)
            return;
        auto& selectedBrush = brushList->data[idx];
        auto itr = std::find_if(Brushes::brushes.begin(), Brushes::brushes.end(), [&](const auto& x)
                                { return x.name == selectedBrush.text; });
        if (itr != Brushes::brushes.end())
        {
            INFO("erasing brush: %s", selectedBrush.text.c_str());
            Brushes::brushes.erase(itr, itr + 1);
            Brushes::Save();
        }

        brushList->tableView->ClearSelection();
        ReloadBrushList();
    }

    void ScribbleViewController::CopyBrush()
    {
        int idx = reinterpret_cast<QuestUI::TableView*>(brushList->tableView)->get_selectedRow();
        INFO("brush copy idx: %d", idx);
        if (idx < 0)
            return;
        auto& selectedBrush = brushList->data[idx];
        auto brush = Brushes::GetBrush(selectedBrush.text);
        if (brush)
        {
            INFO("Got a brush with name: %s", selectedBrush.text.c_str());
            // the brush in our managed list
            auto& brushRef = brush.value().get();
            // duplicate that one
            CustomBrush duplicate = brushRef;

            // if no given name, copy the original name
            if (brushName == "")
                brushName = FindNextName(brushRef.name, 0);
            // if a given name, use that instead
            else
                brushName = brushName = FindNextName(brushName, 0);

            // set name
            duplicate.name = brushName;
            Brushes::brushes.push_back(duplicate);
            // clear name setting
            brushName = "";
            brushNameField->set_text("");
        }

        Brushes::Save();
        ReloadBrushList();
    }

    void ScribbleViewController::SaveBrush()
    {
        int idx = reinterpret_cast<QuestUI::TableView*>(brushList->tableView)->get_selectedRow();
        // we have an index that is selected!;
        std::string selectedBrushName = idx >= 0 ? brushList->data[idx].text : "";

        INFO("saving idx: %d, name: %s", idx, selectedBrushName.c_str());

        // if we want to save, and we have a selected brush name
        if (idx >= 0)
        {
            auto brush = Brushes::GetBrush(selectedBrushName);
            // if the brush existed in the first place
            if (brush)
            {
                auto& brushRef = brush.value().get();
                brushRef.copy(GlobalBrushManager::get_activeBrush()->currentBrush);
                if (brushName != "")
                    brushRef.name = brushName;
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
        brushNameField->set_text("");
        ReloadBrushList();
    }

    void ScribbleViewController::CreateBrushList(UnityEngine::Transform* parent)
    {
        auto brushListVertical = BeatSaberUI::CreateVerticalLayoutGroup(parent);
        brushListVertical->get_gameObject()->GetComponent<Backgroundable*>()->ApplyBackgroundWithAlpha("round-rect-panel", 0.8f);
        //brushListVertical->GetComponent<LayoutElement*>()->set_preferredWidth(40);
        auto brushListHorizontal = BeatSaberUI::CreateHorizontalLayoutGroup(brushListVertical->get_transform());
        brushListHorizontal->set_childForceExpandWidth(false);
        brushListHorizontal->set_childForceExpandHeight(false);
        brushListHorizontal->set_childControlWidth(false);
        brushListHorizontal->set_childControlHeight(false);

        auto nameFieldhorizontal = BeatSaberUI::CreateHorizontalLayoutGroup(brushListVertical->get_transform());
        //nameFieldhorizontal->set_childForceExpandHeight(false);
        //nameFieldhorizontal->GetComponent<LayoutElement*>()->set_preferredHeight(10.0f);
        nameFieldhorizontal->set_padding(RectOffset::New_ctor(0, 0, 2, 2));
        brushNameField = BeatSaberUI::CreateStringSetting(nameFieldhorizontal->get_transform(), "Brush Name", brushName, [this](auto && PH1) { BrushNameChanged(std::forward<decltype(PH1)>(PH1)); });
        brushNameField->GetComponent<LayoutElement*>()->set_preferredWidth(20);

        brushList = BeatSaberUI::CreateScrollableCustomSourceList<CustomBrushListDataSource*>(brushListHorizontal->get_transform(), {45.0f, 50.0f}, [this](auto && PH1) { SelectBrush(std::forward<decltype(PH1)>(PH1)); });

        auto brushSaveButtonsVertical = BeatSaberUI::CreateVerticalLayoutGroup(brushListHorizontal->get_transform());
        float buttonSize = 15.0f;
        brushSaveButtonsVertical->set_childForceExpandHeight(false);
        brushSaveButtonsVertical->set_childForceExpandWidth(false);
        brushSaveButtonsVertical->GetComponent<LayoutElement*>()->set_preferredWidth(buttonSize);

        auto deleteButton = BeatSaberUI::CreateUIButton(brushSaveButtonsVertical->get_transform(), "", "SettingsButton", Vector2(0, 0), Vector2(buttonSize, buttonSize), std::bind(&ScribbleViewController::DeleteBrush, this));
        auto copyButton = BeatSaberUI::CreateUIButton(brushSaveButtonsVertical->get_transform(), "", "SettingsButton", Vector2(0, 0), Vector2(buttonSize, buttonSize), std::bind(&ScribbleViewController::CopyBrush, this));
        auto saveButton = BeatSaberUI::CreateUIButton(brushSaveButtonsVertical->get_transform(), "", "SettingsButton", Vector2(0, 0), Vector2(buttonSize, buttonSize), std::bind(&ScribbleViewController::SaveBrush, this));

        reinterpret_cast<RectTransform*>(deleteButton->get_transform()->GetChild(0))->set_sizeDelta({buttonSize, buttonSize});
        reinterpret_cast<RectTransform*>(copyButton->get_transform()->GetChild(0))->set_sizeDelta({buttonSize, buttonSize});
        reinterpret_cast<RectTransform*>(saveButton->get_transform()->GetChild(0))->set_sizeDelta({buttonSize, buttonSize});

        BeatSaberUI::SetButtonSprites(deleteButton, UITools::Base64ToSprite(trash_inactive), UITools::Base64ToSprite(trash));
        BeatSaberUI::SetButtonSprites(copyButton, UITools::Base64ToSprite(copy_inactive), UITools::Base64ToSprite(copy));
        BeatSaberUI::SetButtonSprites(saveButton, UITools::Base64ToSprite(save_inactive), UITools::Base64ToSprite(save));

        BeatSaberUI::AddHoverHint(deleteButton->get_gameObject(), "Delete the currently selected brush, irreversible!");
        BeatSaberUI::AddHoverHint(copyButton->get_gameObject(), "Duplicate the currently selected brush");
        BeatSaberUI::AddHoverHint(saveButton->get_gameObject(), "Override the currently selected brush with current brush settings");

        ReloadBrushList();
    }

    void ScribbleViewController::CreateMainVertical(UnityEngine::Transform* parent)
    {
        auto rightVertical = BeatSaberUI::CreateVerticalLayoutGroup(parent->get_transform());
        rightVertical->set_childForceExpandWidth(true);
        rightVertical->set_childForceExpandHeight(false);
        auto rightVerticalLayout = rightVertical->GetComponent<LayoutElement*>();
        rightVerticalLayout->set_preferredWidth(92);

        auto listHorizontal = BeatSaberUI::CreateHorizontalLayoutGroup(rightVertical->get_transform());
        auto listHorizontalLayout = listHorizontal->GetComponent<LayoutElement*>();
        listHorizontalLayout->set_preferredWidth(92);

        listHorizontal->set_childForceExpandWidth(false);
        listHorizontal->set_childForceExpandHeight(false);
        listHorizontal->set_childControlWidth(false);
        listHorizontal->set_childControlHeight(false);
        listHorizontal->set_spacing(4);
        CreateBrushList(listHorizontal->get_transform());

        BrushTextures::LoadAllTextures();
        textureList = BeatSaberUI::CreateScrollableList(listHorizontal->get_transform(), {17.0f, 67.0f}, [&](int idx)
                                                        { SelectTexture(idx); });

        textureList->set_listStyle(CustomListTableData::ListStyle::Box);
        textureList->cellSize = 17.0f;
        ReloadTextureList();

        effectList = BeatSaberUI::CreateScrollableList(listHorizontal->get_transform(), {25.0f, 67.0f}, [&](int idx)
                                                       { SelectEffect(idx); });

        effectList->set_listStyle(CustomListTableData::ListStyle::Simple);
        effectList->cellSize = 8.5f;

        ReloadEffectList();

        auto bottomHorizontal = BeatSaberUI::CreateHorizontalLayoutGroup(rightVertical->get_transform());
        auto bottomHorizontalLayout = bottomHorizontal->GetComponent<LayoutElement*>();
        bottomHorizontalLayout->set_preferredWidth(92);
        bottomHorizontal->set_childForceExpandWidth(true);

        auto bottomVertical = BeatSaberUI::CreateVerticalLayoutGroup(bottomHorizontal->get_transform());
        bottomVertical->set_childForceExpandHeight(false);
        bottomVertical->set_childForceExpandWidth(true);
        bottomVertical->set_childControlHeight(false);
        bottomVertical->set_childControlWidth(true);
        bottomVertical->set_spacing(2);

        auto brush = GlobalBrushManager::get_activeBrush();
        if (brush)
            size = brush->currentBrush.size;
        //    <slider-setting id='SizeSlider' text='Brush Size' min='1' max='70' increment='1' integer-only='false' apply-on-change='true' value='Size' />
        sizeSlider = BeatSaberUI::CreateSliderSetting(bottomVertical->get_transform(), "Brush Size", 1.0f, size, 1.0f, 70.0f, 0.5f, [&](float val)
                                                      {
                                                          size = (int)val;
                                                          auto brush = GlobalBrushManager::get_activeBrush();
                                                          if (brush)
                                                              brush->currentBrush.size = size;
                                                      });

        //    <slider-setting id='GlowSlider' text='Glow Amount' min='0' max='1' increment='0.05' integer-only='false' apply-on-change='true' value='Glow' />
        if (brush)
            glow = brush->currentBrush.glow;
        glowSlider = BeatSaberUI::CreateSliderSetting(bottomVertical->get_transform(), "Glow Amount", 0.05f, glow, 0.0f, 1.0f, 0.5f, [&](float val)
                                                      {
                                                          glow = val;
                                                          auto brush = GlobalBrushManager::get_activeBrush();
                                                          if (brush)
                                                              brush->currentBrush.glow = glow;
                                                      });

        //    <slider-setting id='GlowSlider' text='Glow Amount' min='0' max='1' increment='0.05' integer-only='false' apply-on-change='true' value='Glow' />
        if (brush)
            tile = brush->currentBrush.tiling.x;
        tileSlider = BeatSaberUI::CreateSliderSetting(bottomVertical->get_transform(), "Tiling", 1.0f, tile, 1.0f, 20.0f, 0.5f, [&](float val)
                                                      {
                                                          tile = val;
                                                          auto brush = GlobalBrushManager::get_activeBrush();
                                                          if (brush)
                                                              brush->currentBrush.tiling.x = tile;
                                                      });
    }

    void ScribbleViewController::CreateLeftToolBar(UnityEngine::Transform* parent)
    {
        auto verticalTop = BeatSaberUI::CreateVerticalLayoutGroup(parent->get_transform());
        verticalTop->set_padding(UnityEngine::RectOffset::New_ctor(0, 0, 5, 0));
        verticalTop->set_spacing(2);
        auto layoutTop = verticalTop->get_gameObject()->AddComponent<LayoutElement*>();
        layoutTop->set_preferredWidth(12);
        verticalTop->set_childControlHeight(true);
        verticalTop->set_childControlWidth(true);
        verticalTop->set_childForceExpandHeight(false);
        verticalTop->set_childForceExpandWidth(false);
        auto toolBarVertical = BeatSaberUI::CreateVerticalLayoutGroup(verticalTop->get_transform());

        auto layoutButtons = verticalTop->get_gameObject()->AddComponent<LayoutElement*>();
        toolBarVertical->GetComponent<ContentSizeFitter*>()->set_verticalFit(ContentSizeFitter::FitMode::PreferredSize);
        float buttonSize = 20;
        toolBarVertical->set_spacing(2);
        layoutButtons->set_preferredWidth(buttonSize);
        toolBarVertical->set_childControlHeight(true);
        toolBarVertical->set_childControlWidth(true);
        toolBarVertical->set_childForceExpandHeight(false);
        toolBarVertical->set_childForceExpandWidth(false);

        auto pickerButton = BeatSaberUI::CreateUIButton(toolBarVertical->get_transform(), "", "SettingsButton", Vector2(0, 0), Vector2(buttonSize, buttonSize), std::bind(&ScribbleViewController::SelectPicker, this));
        reinterpret_cast<RectTransform*>(pickerButton->get_transform()->GetChild(0))->set_sizeDelta({buttonSize, buttonSize});
        pickerImage = pickerButton->GetComponentInChildren<HMUI::ImageView*>();

        //<button-with-icon id="picker-btn" stroke-type="Clean" preferred-width="12" preferred-height="12" on-click="selectPicker" click-event="show-picker" hover-hint="Color Picker" />
        auto eraserButton = BeatSaberUI::CreateUIButton(toolBarVertical->get_transform(), "", "SettingsButton", Vector2(0, 0), Vector2(buttonSize, buttonSize), std::bind(&ScribbleViewController::SelectEraseMode, this));
        reinterpret_cast<RectTransform*>(eraserButton->get_transform()->GetChild(0))->set_sizeDelta({buttonSize, buttonSize});
        eraserImage = eraserButton->GetComponentInChildren<HMUI::ImageView*>();

        auto bucketButton = BeatSaberUI::CreateUIButton(toolBarVertical->get_transform(), "", "SettingsButton", Vector2(0, 0), Vector2(buttonSize, buttonSize), std::bind(&ScribbleViewController::SelectBucketMode, this));
        reinterpret_cast<RectTransform*>(bucketButton->get_transform()->GetChild(0))->set_sizeDelta({buttonSize, buttonSize});
        bucketImage = bucketButton->GetComponentInChildren<HMUI::ImageView*>();

        auto rulerButton = BeatSaberUI::CreateUIButton(toolBarVertical->get_transform(), "", "SettingsButton", Vector2(0, 0), Vector2(buttonSize, buttonSize), std::bind(&ScribbleViewController::SelectRulerMode, this));
        reinterpret_cast<RectTransform*>(rulerButton->get_transform()->GetChild(0))->set_sizeDelta({buttonSize, buttonSize});
        rulerImage = rulerButton->GetComponentInChildren<HMUI::ImageView*>();

        auto moverButton = BeatSaberUI::CreateUIButton(toolBarVertical->get_transform(), "", "SettingsButton", Vector2(0, 0), Vector2(buttonSize, buttonSize), std::bind(&ScribbleViewController::SelectMoverMode, this));
        reinterpret_cast<RectTransform*>(moverButton->get_transform()->GetChild(0))->set_sizeDelta({buttonSize, buttonSize});
        moveImage = moverButton->GetComponentInChildren<HMUI::ImageView*>();

        BeatSaberUI::SetButtonSprites(pickerButton, UITools::Base64ToSprite(picker_inactive), UITools::Base64ToSprite(picker));
        BeatSaberUI::SetButtonSprites(eraserButton, UITools::Base64ToSprite(eraser_inactive), UITools::Base64ToSprite(eraser));
        BeatSaberUI::SetButtonSprites(bucketButton, UITools::Base64ToSprite(bucket_inactive), UITools::Base64ToSprite(bucket));
        BeatSaberUI::SetButtonSprites(rulerButton, UITools::Base64ToSprite(ruler_inactive), UITools::Base64ToSprite(ruler));
        BeatSaberUI::SetButtonSprites(moverButton, UITools::Base64ToSprite(move_inactive), UITools::Base64ToSprite(move));

        UnityEngine::Color color = {0, 0, 0, 1.0};
        if (GlobalBrushManager::get_activeBrush())
            color = GlobalBrushManager::get_activeBrush()->currentBrush.color;
        colorPickerModal = BeatSaberUI::CreateColorPickerModal(get_transform(), "", color, std::bind(&ScribbleViewController::PickerSelectedColor, this, std::placeholders::_1));

        colorHistoryPanel = ColorHistoryPanelController::CreateColorHistoryPanel(colorPickerModal->modalView->get_transform(), Vector2(10, 50), [&](Sombrero::FastColor color)
                                                                                {
                                                                                    colorPickerModal->set_color(color);
                                                                                });
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
        if (!textureList)
            return;
        textureList->data.clear();
        int row = reinterpret_cast<QuestUI::TableView*>(textureList->tableView)->get_selectedRow();
        auto textures = BrushTextures::GetTextures();
        for (auto& t : textures)
        {
            auto texture = t.second;
            if (texture)
            {
                auto sprite = Sprite::Create(texture, UnityEngine::Rect(0.0f, 0.0f, (float)texture->get_width(), (float)texture->get_height()), UnityEngine::Vector2(0.5f, 0.5f), 1024.0f, 1u, UnityEngine::SpriteMeshType::FullRect, UnityEngine::Vector4(0.0f, 0.0f, 0.0f, 0.0f), false);
                Object::DontDestroyOnLoad(sprite);
                textureList->data.emplace_back(t.first, sprite);
            }
            else
                textureList->data.emplace_back(t.first);
        }

        textureList->tableView->ReloadData();
        textureList->tableView->RefreshCells(true, true);
        textureList->tableView->ScrollToCellWithIdx(row >= 0 ? row : 0, HMUI::TableView::ScrollPositionType::Beginning, false);
    }

    void ScribbleViewController::ReloadEffectList()
    {
        INFO("ReloadEffectList");
        if (!effectList)
            return;
        effectList->data.clear();
        int row = reinterpret_cast<QuestUI::TableView*>(effectList->tableView)->get_selectedRow();
        auto& effects = Effects::GetEffects();
        for (auto& e : effects)
        {
            effectList->data.emplace_back(e->get_name());
        }
        effectList->tableView->ReloadData();
        effectList->tableView->RefreshCells(true, true);
        effectList->tableView->ScrollToCellWithIdx(row >= 0 ? row : 0, HMUI::TableView::ScrollPositionType::Beginning, false);
    }

    void ScribbleViewController::ReloadBrushList()
    {
        INFO("ReloadBrushList");
        if (!brushList)
            return;
        brushList->data.clear();
        int firstRow = brushList->tableView->get_contentTransform() ? reinterpret_cast<QuestUI::TableView*>(brushList->tableView)->get_scrolledRow() : 0;

        for (auto b : Brushes::brushes)
        {
            INFO("Brush: %s", b.name.c_str());
            auto texture = BrushTextures::GetTexture(b.textureName);
            if (texture)
            {
                auto sprite = Sprite::Create(texture, UnityEngine::Rect(0.0f, 0.0f, (float)texture->get_width(), (float)texture->get_height()), UnityEngine::Vector2(0.5f, 0.5f), 1024.0f, 1u, UnityEngine::SpriteMeshType::FullRect, UnityEngine::Vector4(0.0f, 0.0f, 0.0f, 0.0f), false);
                Object::DontDestroyOnLoad(sprite);
                brushList->data.emplace_back(b.name, b.effectName, b.color, sprite);
            }
            else
            {
                brushList->data.emplace_back(b.name, b.effectName, b.color);
            }
        }

        brushList->tableView->ReloadData();
        brushList->tableView->RefreshCells(true, true);
        brushList->tableView->ScrollToCellWithIdx(firstRow > 0 ? firstRow : 0, HMUI::TableView::ScrollPositionType::Beginning, true);
    }

    void ScribbleViewController::SelectForBrush(const CustomBrush& brush)
    {
        int idx = 0;
        for (auto t : textureList->data)
        {
            if (t.text == brush.textureName)
                break;
            idx++;
        }

        textureList->tableView->ScrollToCellWithIdx(idx, HMUI::TableView::ScrollPositionType::Beginning, true);
        textureList->tableView->SelectCellWithIdx(idx, false);

        idx = 0;
        for (auto t : effectList->data)
        {
            if (t.text == brush.effectName)
                break;
            idx++;
        }

        effectList->tableView->ScrollToCellWithIdx(idx, HMUI::TableView::ScrollPositionType::Beginning, true);
        effectList->tableView->SelectCellWithIdx(idx, false);

        glowSlider->set_value(brush.glow);
        sizeSlider->set_value(brush.size);
        tileSlider->set_value(brush.tiling.x);
        colorPickerModal->set_color(brush.color);
        pickerImage->set_color(brush.color);
        brushNameField->set_text(brush.name);
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
            if (t.text == newBrush->currentBrush.name)
                break;
            idx++;
        }

        brushList->tableView->ScrollToCellWithIdx(idx, HMUI::TableView::ScrollPositionType::Beginning, true);
        brushList->tableView->SelectCellWithIdx(idx, false);

        UpdateBrushModeButtonsColors(newBrush);
    }

    void ScribbleViewController::UpdateBrushModeButtonsColors(BrushBehaviour* brushBehaviour)
    {
        eraserImage->set_color(brushBehaviour->mode == BrushBehaviour::BrushMode::Erase ? Color(1.0, 0.0, 0.0, 1.0) : Color(1.0, 1.0, 1.0, 1.0));
        bucketImage->set_color(brushBehaviour->mode == BrushBehaviour::BrushMode::Bucket ? Color(0.0, 0.0, 1.0, 1.0) : Color(1.0, 1.0, 1.0, 1.0));
        rulerImage->set_color(brushBehaviour->mode == BrushBehaviour::BrushMode::Ruler ? Color(0.0, 1.0, 0.0, 1.0) : Color(1.0, 1.0, 1.0, 1.0));
        moveImage->set_color(brushBehaviour->mode == BrushBehaviour::BrushMode::Move ? Color(0.55, 0.25, 0.75, 1.0) : Color(1.0, 1.0, 1.0, 1.0));
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
        if (colorPickerModal)
            colorPickerModal->Show();
    }

    void ScribbleViewController::PickerSelectedColor(Color color)
    {
        auto brush = GlobalBrushManager::get_activeBrush();
        if (brush)
        {
            brush->currentBrush.color = color;
            pickerImage->set_color(color);
            colorHistoryPanel->AddColor(color);
        }
    }

    void ScribbleViewController::SelectMoverMode()
    {
        auto brush = GlobalBrushManager::get_activeBrush();
        if (brush)
        {
            brush->mode = brush->mode == BrushBehaviour::BrushMode::Move ? BrushBehaviour::BrushMode::Brush : BrushBehaviour::BrushMode::Move;
            UpdateBrushModeButtonsColors(brush);
        }
    }

    void ScribbleViewController::SelectRulerMode()
    {
        auto brush = GlobalBrushManager::get_activeBrush();
        if (brush)
        {
            brush->mode = brush->mode == BrushBehaviour::BrushMode::Ruler ? BrushBehaviour::BrushMode::Brush : BrushBehaviour::BrushMode::Ruler;
            UpdateBrushModeButtonsColors(brush);
        }
    }

    void ScribbleViewController::SelectBucketMode()
    {
        auto brush = GlobalBrushManager::get_activeBrush();
        if (brush)
        {
            brush->mode = brush->mode == BrushBehaviour::BrushMode::Bucket ? BrushBehaviour::BrushMode::Brush : BrushBehaviour::BrushMode::Bucket;
            UpdateBrushModeButtonsColors(brush);
        }
    }

    void ScribbleViewController::SelectEraseMode()
    {
        auto brush = GlobalBrushManager::get_activeBrush();
        if (brush)
        {
            brush->mode = brush->mode == BrushBehaviour::BrushMode::Erase ? BrushBehaviour::BrushMode::Brush : BrushBehaviour::BrushMode::Erase;
            UpdateBrushModeButtonsColors(brush);
        }
    }

    void ScribbleViewController::ShowSaveFile()
    {
        if (saveModal)
            saveModal->Show(true, true, nullptr);
    }

    void ScribbleViewController::ShowLoadFile()
    {
        if (loadModal)
            loadModal->Show(true, true, nullptr);
    }

    void ScribbleViewController::SaveFilenameChanged(std::string val)
    {
        saveFileName = val;
    }

    void ScribbleViewController::BrushNameChanged(std::string val)
    {
        brushName = val;
    }

    void ScribbleViewController::ReloadFileLists() const
    {
        std::vector<std::string> fileNames = {};
        FileUtils::GetFilesInFolderPath("png", drawingPath, fileNames);

        for (auto& f : fileNames)
        {
            std::string name = FileUtils::GetFileName(f, true);

            bool addSave = std::find_if(saveFileList->data.begin(), saveFileList->data.end(), [&](const auto& x)
                                        { return x.text == name; }) == saveFileList->data.end();
            bool addLoad = std::find_if(loadFileList->data.begin(), loadFileList->data.end(), [&](const auto& x)
                                        { return x.text == name; }) == loadFileList->data.end();

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

                auto sprite = texture ? Sprite::Create(texture, UnityEngine::Rect(0.0f, 0.0f, (float)texture->get_width(), (float)texture->get_height()), UnityEngine::Vector2(0.5f, 0.5f), 1024.0f, 1u, UnityEngine::SpriteMeshType::FullRect, UnityEngine::Vector4(0.0f, 0.0f, 0.0f, 0.0f), false) : nullptr;
                if (addSave)
                    saveFileList->data.emplace_back(name, sprite);
                if (addLoad)
                    loadFileList->data.emplace_back(name, sprite);
            }
        }

        saveFileList->tableView->ReloadData();
        loadFileList->tableView->ReloadData();

        // models list
        modelFileList->tableView->RefreshCells(true, true);

        fileNames.clear();
        FileUtils::GetFilesInFolderPath("obj", modelsPath, fileNames);

        for (auto& f : fileNames)
        {
            std::string name = FileUtils::GetFileName(f, true);

            bool addLoad = std::find_if(modelFileList->data.begin(), modelFileList->data.end(), [&](const auto& x)
                                        { return x.text == name; }) == modelFileList->data.end();

            if (addLoad)
            {
                std::ifstream reader(string_format("%s/%s", drawingPath, f.c_str()), std::ios::in | std::ios::binary);
                long size;
                UnityEngine::Texture2D* texture = nullptr;

                if (ThumbnailHelper::CheckPngData(reader, size, false)) // if success
                {
                    INFO("Reading texture from ifstream");
                    texture = ThumbnailHelper::ReadPNG(reader, size);
                }

                if (addLoad)
                    modelFileList->data.emplace_back(name, nullptr);
            }
        }

        modelFileList->tableView->ReloadData();

        modelFileList->tableView->RefreshCells(true, true);
    }

    void ScribbleViewController::SaveSelectIdx(int idx)
    {
        fileNameField->set_text(saveFileList->data[idx].text);
    }

    void ScribbleViewController::ShowSettings()
    {
        if (settingsModal)
            settingsModal->Show(true, true, nullptr);
    }

    void ScribbleViewController::ShowModels()
    {
        if (modelsModal)
            modelsModal->Show(true, true, nullptr);
    }
}