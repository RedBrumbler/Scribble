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

#include "HMUI/Screen.hpp"
#include "HMUI/CurvedCanvasSettings.hpp"

#include "UI/UITools.hpp"

#include "Zenject/MonoInstaller.hpp"

#include "questui/shared/BeatSaberUI.hpp"

#include "config.hpp"

#include "ScribbleContainer.hpp"

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

        LoadMainView();

        auto startButton = UITools::CreateSimpleButton(rectTransform, "Start Drawing");
        startButton.SetAnchor(0.5f, 0.5f);
        startButton.SetSize(30, 10);
        startButton.SetPosition(0, -51);
        startButton.AddListener([startButton, this]()
        {
            config.drawingEnabled ^= 1;
            SaveConfig();
            auto btn = const_cast<ScribbleUISimpleButton*>(&startButton);
            btn->set_text(config.drawingEnabled ? "Stop Drawing" : "Start Drawing");
            
            if (config.firstTimeLaunch && !pressedToggleBefore)
            {
                config.firstTimeLaunch = true;
                SaveConfig();
                pressedToggleBefore = true;
                //TODO
                //ScribbleContainer::get_instance().LoadAnimated("second", 0.004f, true);
            }
        });

        startButton.gameObject->SetActive(true);

        CreateLogo();

        CreateToolBar();

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
        /*
        var img = imageContainer.AddComponent<Image>();
        img.material = _uiMaterial;
        img.type = Image.Type.Sliced;
        */
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
        toolBar->set_sizeDelta(Vector2(200, 10));

        auto sizeFitter = topToolbar->AddComponent<ContentSizeFitter*>();
        sizeFitter->set_horizontalFit(ContentSizeFitter::FitMode::PreferredSize);

        auto horizontalLayout = topToolbar->AddComponent<HorizontalLayoutGroup*>();
        horizontalLayout->set_childControlWidth(false);
        horizontalLayout->set_childAlignment(TextAnchor::UpperLeft);
        horizontalLayout->set_spacing(5.0f);

        auto btn = UITools::CreateSimpleButton(toolBar, "Clear");
        btn.strokeEnabled = false;
        btn.AddListener([]() { if (ScribbleContainer::get_instance() && !ScribbleContainer::get_instance()->get_IsInAnimation()) ScribbleContainer::get_instance()->Clear(); });
        btn.gameObject->SetActive(true);

        btn = UITools::CreateSimpleButton(toolBar, "Undo");
        btn.strokeEnabled = false;
        btn.AddListener([]() { if (ScribbleContainer::get_instance() && !ScribbleContainer::get_instance()->get_IsInAnimation()) ScribbleContainer::get_instance()->Undo(); });
        btn.gameObject->SetActive(true);

        btn = UITools::CreateSimpleButton(toolBar, "Save");
        btn.strokeEnabled = false;
        btn.AddListener([&]() { /*mainViewController->ShowSaveFile();*/ });
        btn.gameObject->SetActive(true);

        btn = UITools::CreateSimpleButton(toolBar, "Load");
        btn.strokeEnabled = false;
        btn.AddListener([&]() { /*mainViewController->ShowLoadFile();*/ });
        btn.gameObject->SetActive(true);
    }

    void ScribbleUI::CreateLogo()
    {
        /*
        var img = _globalContainer.CreateImage(new Vector2(20, -12), new Vector2(34, 14));
        img.rectTransform.anchorMin = new Vector2(0, 1);
        img.rectTransform.anchorMax = new Vector2(0, 1);
        img.sprite = Tools.LoadSpriteFromResources("Resources.logo.png", false);
        */
    }

    void ScribbleUI::LoadMainView()
    {
        mainViewController = QuestUI::BeatSaberUI::CreateViewController<ScribbleViewController*>();
        auto curvedCanvasSettings = mainViewController->GetComponentsInChildren<HMUI::CurvedCanvasSettings*>();
        int length = curvedCanvasSettings->Length();
        for ( int i = 0; i < length; i++)
            curvedCanvasSettings->values[i]->SetRadius(0);

        get_gameObject()->GetComponent<HMUI::Screen*>()->rootViewController = mainViewController;
        mainViewController->get_transform()->SetParent(globalContainer->get_parent(), false);
        mainViewController->get_transform()->set_localPosition({0.0f, -0.1f, -0.01f});
        mainViewController->get_transform()->set_localRotation(Quaternion::Euler(-5.0f, 0.0f, 0.0f));
        mainViewController->__Activate(true, true);
    }
}