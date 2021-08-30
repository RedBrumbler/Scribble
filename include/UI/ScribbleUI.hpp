#pragma once

#include "custom-types/shared/macros.hpp"
#include "UnityEngine/MonoBehaviour.hpp"
#include "UnityEngine/Canvas.hpp"
#include "UnityEngine/RectTransform.hpp"

#include "UI/ScribbleViewController.hpp"
#include "Zenject/DiContainer.hpp"
#include "UI/UITools.hpp"

DECLARE_CLASS_CODEGEN(Scribble, ScribbleUI, UnityEngine::MonoBehaviour,
    DECLARE_INSTANCE_FIELD(ScribbleViewController*, mainViewController);
    DECLARE_INSTANCE_FIELD(UnityEngine::Canvas*, canvas);
    DECLARE_INSTANCE_FIELD(UnityEngine::RectTransform*, rectTransform);
    DECLARE_INSTANCE_FIELD(UnityEngine::RectTransform*, globalContainer);
    DECLARE_INSTANCE_FIELD(UnityEngine::RectTransform*, toolBar);
    DECLARE_INSTANCE_METHOD(void, Awake);

    public:
        static ScribbleUI* Create();
        void Show(bool shouldShow = true);
        static void Reset();
        ScribbleUISimpleButton startButton;
        static inline bool inPause = false;
    private:
        bool pressedToggleBefore = false;
        void CreateContainer();
        void CreateToolBar();
        void CreateLogo();
        void LoadMainView();
        static void SetMainScreenInteractable(bool interactable);
)