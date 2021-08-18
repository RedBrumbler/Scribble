#pragma once

#include "custom-types/shared/macros.hpp"
#include "UnityEngine/MonoBehaviour.hpp"
#include "UnityEngine/GameObject.hpp"
#include "CustomTypes/InputManager.hpp"

DECLARE_CLASS_CODEGEN(Scribble, BrushBehaviour, UnityEngine::MonoBehaviour,
    DECLARE_INSTANCE_FIELD(Scribble::InputManager*, inputManager);
    DECLARE_INSTANCE_FIELD(UnityEngine::GameObject*, obj);
    DECLARE_INSTANCE_METHOD(void, Update);
    DECLARE_INSTANCE_METHOD(void, Start);
    public:
        GlobalNamespace::SaberType saberType;
    private:
        void OnPress();
        void OnRelease();
)