#include "UI/ScribbleUI.hpp"

#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Object.hpp"
#include "UnityEngine/Vector2.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Quaternion.hpp"
#include "UnityEngine/Canvas.hpp"
#include "UnityEngine/RenderMode.hpp"
#include "UnityEngine/Camera.hpp"
#include "UnityEngine/UI/ContentSizeFitter.hpp"
#include "UnityEngine/UI/CanvasScaler.hpp"
#include "UnityEngine/TextAnchor.hpp"
#include "UnityEngine/UI/HorizontalLayoutGroup.hpp"

#include "VRUIControls/VRGraphicRaycaster.hpp"

#include "HMUI/TitleViewController.hpp"
#include "HMUI/HierarchyManager.hpp"
#include "HMUI/ScreenSystem.hpp"
#include "HMUI/Screen.hpp"
#include "HMUI/CurvedCanvasSettings.hpp"

#include "UI/UITools.hpp"

#include "Zenject/MonoInstaller.hpp"

#include "questui/shared/BeatSaberUI.hpp"

#include "config.hpp"

#include "ScribbleContainer.hpp"

#include "static-defines.hpp"
#include "icons.hpp"

DEFINE_TYPE(Scribble, ScribbleUI);

using namespace UnityEngine;
using namespace UnityEngine::UI;

namespace Scribble
{
    void ScribbleUI::Awake()
    {
        get_gameObject()->SetActive(false);

        //backgroundSprite = Resources.FindObjectsOfTypeAll<Sprite>().FirstOrDefault(s => s.name == "Background");

        /*
        var shader = AssetLoader.LoadAsset<Shader>("assets/shaders/UIShader.shader");
        _uiMaterial = new Material(shader);
        _uiMaterial.SetColor("_Color", new Color(0.05f, 0.05f, 0.05f));
        _uiMaterial.SetColor("_GlowColor", new Color(3/255f, 152/255f, 252/255f));
        _uiMaterial.SetTexture("_Map", Tools.LoadTextureFromResources("Resources.glowmask.png"));
        */

        get_transform()->set_position(Vector3(0, 0.1f, 0.99f));
        get_transform()->set_eulerAngles(Vector3(90, 0, 0));
        get_transform()->set_localScale(Vector3(0.015f, 0.015f, 0.015f));
        canvas = get_gameObject()->AddComponent<Canvas*>();
        canvas->set_renderMode(RenderMode::WorldSpace);
        canvas->set_worldCamera(Camera::get_main());
        get_gameObject()->AddComponent<VRUIControls::VRGraphicRaycaster*>()->physicsRaycaster = QuestUI::BeatSaberUI::GetPhysicsRaycasterWithCache();

        rectTransform = reinterpret_cast<RectTransform*>(canvas->get_transform());
        rectTransform->set_sizeDelta(Vector2(250, 150));

        auto canvasScaler = get_gameObject()->AddComponent<CanvasScaler*>();
        canvasScaler->set_referencePixelsPerUnit(10.0f);
        canvasScaler->set_dynamicPixelsPerUnit(3.44f);

        auto curved = get_gameObject()->AddComponent<HMUI::CurvedCanvasSettings*>();
        curved->SetRadius(0);

        auto screen = get_gameObject()->AddComponent<HMUI::Screen*>();

        //TODO
        //_canvas.transform.SetParent(GameObject.Find("ScreenSystem").transform.parent);

        CreateContainer();

        auto startButton = UITools::CreateSimpleButton(rectTransform, "Start Drawing");
        startButton.SetAnchor(0.5f, 0.5f);
        startButton.SetSize(30, 10);
        startButton.SetPosition(0, -51);
        startButton.AddListener([startButton, this]()
        {
            if (!mainViewController)
            {
                LoadMainView();
            }

            bool on = (ScribbleContainer::get_instance()->drawingEnabled ^= 1);
            globalContainer->get_gameObject()->SetActive(on);
            SaveConfig();
            auto btn = const_cast<ScribbleUISimpleButton*>(&startButton);
            btn->set_text(on ? "Stop Drawing" : "Start Drawing");
            
            if (config.firstTimeLaunch && !pressedToggleBefore)
            {
                config.firstTimeLaunch = false;
                SaveConfig();
                pressedToggleBefore = true;
                //TODO
                ScribbleContainer::get_instance()->Load(string_format("%s/%s", drawingPath, "second.png"), true, true);
            }

            SetMainScreenInteractable(!on);
        });

        startButton.gameObject->SetActive(true);

        CreateToolBar();
        CreateLogo();

        globalContainer->get_gameObject()->SetActive(false);
        get_gameObject()->SetActive(true);
    }

    ScribbleUI* ScribbleUI::Create()
    {
        auto go = GameObject::New_ctor(il2cpp_utils::newcsstr("ScribbleUI"));
        Object::DontDestroyOnLoad(go);
        return go->AddComponent<ScribbleUI*>(); 
    }

    void ScribbleUI::Show(bool shouldShow)
    {
        get_gameObject()->SetActive(shouldShow);
    }

    void ScribbleUI::CreateContainer()
    {
        auto imageContainer = GameObject::New_ctor(il2cpp_utils::newcsstr("ImageContainer"));
        imageContainer->get_transform()->SetParent(rectTransform, false);
        auto rect = imageContainer->AddComponent<RectTransform*>();
        rect->set_anchoredPosition3D(Vector3(0, 0, 0.01f));
        rect->set_anchorMin(Vector2(0, 0));
        rect->set_anchorMax(Vector2(1, 1));
        rect->set_pivot(Vector2(0.5f, 0.5f));
        rect->set_offsetMin(Vector2(0, 0));
        rect->set_offsetMax(Vector2(0, 0));
        
        auto container = GameObject::New_ctor(il2cpp_utils::newcsstr("Container"));
        container->get_transform()->SetParent(rect, false);
        globalContainer = container->AddComponent<RectTransform*>();
        globalContainer->set_anchoredPosition3D(Vector3(0, 0, -0.01f));
        globalContainer->set_anchorMin(Vector2(0, 0));
        globalContainer->set_anchorMax(Vector2(1, 1));
        globalContainer->set_pivot(Vector2(0.5f, 0.5f));
        globalContainer->set_offsetMin(Vector2(0, 0));
        globalContainer->set_offsetMax(Vector2(0, 0));
    }

    void ScribbleUI::CreateToolBar()
    {
        auto topToolbar = GameObject::New_ctor(il2cpp_utils::newcsstr("Top_Toolbar"));
        topToolbar->get_transform()->SetParent(globalContainer, false);
        toolBar = topToolbar->AddComponent<RectTransform*>();
        toolBar->set_anchoredPosition3D(Vector3(0, 0, -2));
        toolBar->set_anchorMin(Vector2(0, 0.5f));
        toolBar->set_anchorMax(Vector2(1, 0.5f));
        toolBar->set_pivot(Vector2(0.5f, 0.5f));
        toolBar->set_offsetMin(Vector2(0, 65));
        toolBar->set_offsetMax(Vector2(0, 65));
        toolBar->set_localRotation(Quaternion::Euler(-30, 0, 0));
        toolBar->set_sizeDelta(Vector2(150, 10));

        auto sizeFitter = topToolbar->AddComponent<ContentSizeFitter*>();
        sizeFitter->set_horizontalFit(ContentSizeFitter::FitMode::PreferredSize);
        auto topBarLayout = topToolbar->AddComponent<HorizontalLayoutGroup*>();
        topBarLayout->set_childAlignment(TextAnchor::UpperLeft);
        topBarLayout->set_spacing(5);
        //topBarLayout->set_childControlWidth(false);
        //topBarLayout->set_childForceExpandWidth(false);

        auto horizontalLayout = QuestUI::BeatSaberUI::CreateHorizontalLayoutGroup(toolBar);
        horizontalLayout->set_childForceExpandWidth(false);
        horizontalLayout->set_childControlWidth(false);
        horizontalLayout->set_spacing(2.0f);
        horizontalLayout->GetComponent<UI::LayoutElement*>()->set_preferredWidth(80);
        
        auto btn = QuestUI::BeatSaberUI::CreateUIButton(horizontalLayout->get_transform(), "Clear", [](){ if (ScribbleContainer::get_instance() && !ScribbleContainer::get_instance()->get_IsInAnimation()) ScribbleContainer::get_instance()->Clear(); });
        btn = QuestUI::BeatSaberUI::CreateUIButton(horizontalLayout->get_transform(), "Undo", [](){ if (ScribbleContainer::get_instance() && !ScribbleContainer::get_instance()->get_IsInAnimation()) ScribbleContainer::get_instance()->Undo(); });
        btn = QuestUI::BeatSaberUI::CreateUIButton(horizontalLayout->get_transform(), "Save", [&]() { mainViewController->ShowSaveFile(); });
        btn = QuestUI::BeatSaberUI::CreateUIButton(horizontalLayout->get_transform(), "Load", [&]() { mainViewController->ShowLoadFile(); });
        /*
        auto btn = UITools::CreateSimpleButton(horizontalLayout->get_transform(), "Clear");
        btn.strokeEnabled = false;
        btn.AddListener([]() { if (ScribbleContainer::get_instance() && !ScribbleContainer::get_instance()->get_IsInAnimation()) ScribbleContainer::get_instance()->Clear(); });
        btn.gameObject->SetActive(true);

        btn = UITools::CreateSimpleButton(horizontalLayout->get_transform(), "Undo");
        btn.strokeEnabled = false;
        btn.AddListener([]() { if (ScribbleContainer::get_instance() && !ScribbleContainer::get_instance()->get_IsInAnimation()) ScribbleContainer::get_instance()->Undo(); });
        btn.gameObject->SetActive(true);

        btn = UITools::CreateSimpleButton(horizontalLayout->get_transform(), "Save");
        btn.strokeEnabled = false;
        btn.AddListener([&]() { mainViewController->ShowSaveFile(); });
        btn.gameObject->SetActive(true);

        btn = UITools::CreateSimpleButton(horizontalLayout->get_transform(), "Load");
        btn.strokeEnabled = false;
        btn.AddListener([&]() { mainViewController->ShowLoadFile(); });
        btn.gameObject->SetActive(true);
        */

        auto settingsButton = QuestUI::BeatSaberUI::CreateUIButton(toolBar->get_transform(), "", "SettingsButton", Vector2(8, 8), [&](){ mainViewController->ShowSettings(); });
        reinterpret_cast<RectTransform*>(settingsButton->get_transform()->GetChild(0))->set_sizeDelta({8, 8});

        QuestUI::BeatSaberUI::SetButtonSprites(settingsButton, UITools::Base64ToSprite(settings_inactive), UITools::Base64ToSprite(settings));
    }

    void ScribbleUI::CreateLogo()
    {
        auto horizontal = QuestUI::BeatSaberUI::CreateHorizontalLayoutGroup(toolBar);
        horizontal->get_transform()->SetAsFirstSibling();
        auto layout = horizontal->GetComponent<LayoutElement*>();
        layout->set_preferredWidth(20);
        layout->set_preferredHeight(8);
        auto img = UITools::CreateImage(horizontal->get_transform(), Vector2(0, -12), Vector2(34, 14));
        img->get_rectTransform()->set_anchorMin(Vector2(0, 1));
        img->get_rectTransform()->set_anchorMax(Vector2(0, 1));
        
        img->set_sprite(UITools::Base64ToSprite(logo));
    }

    void ScribbleUI::LoadMainView()
    {
        mainViewController = QuestUI::BeatSaberUI::CreateViewController<ScribbleViewController*>();
        auto curvedCanvasSettings = mainViewController->GetComponentsInChildren<HMUI::CurvedCanvasSettings*>();
        int length = curvedCanvasSettings->Length();
        for ( int i = 0; i < length; i++)
            curvedCanvasSettings->values[i]->SetRadius(0);

        get_gameObject()->GetComponent<HMUI::Screen*>()->rootViewController = mainViewController;
        mainViewController->get_transform()->SetParent(globalContainer, false);
        mainViewController->get_transform()->set_localPosition({0.0f, -0.1f, -0.01f});
        mainViewController->__Activate(true, true);
    }

    void ScribbleUI::SetMainScreenInteractable(bool interactable)
    {
        auto hierarchyManagers = Resources::FindObjectsOfTypeAll<HMUI::HierarchyManager*>();
        HMUI::ScreenSystem* screenSystem = (hierarchyManagers && hierarchyManagers->Length() > 0) ? hierarchyManagers->values[0]->screenSystem : nullptr;
        if (!screenSystem) return;

        if (screenSystem->get_mainScreen() && screenSystem->get_mainScreen()->rootViewController) screenSystem->get_mainScreen()->rootViewController->set_enableUserInteractions(interactable);
        if (screenSystem->get_leftScreen() && screenSystem->get_leftScreen()->rootViewController) screenSystem->get_leftScreen()->rootViewController->set_enableUserInteractions(interactable);
        if (screenSystem->get_rightScreen() && screenSystem->get_rightScreen()->rootViewController) screenSystem->get_rightScreen()->rootViewController->set_enableUserInteractions(interactable);
        if (screenSystem->get_topScreen() && screenSystem->get_topScreen()->rootViewController) screenSystem->get_topScreen()->rootViewController->set_enableUserInteractions(interactable);
        if (screenSystem->get_titleViewController() && screenSystem->get_titleViewController()) screenSystem->get_titleViewController()->set_enableUserInteractions(interactable);
    }
}