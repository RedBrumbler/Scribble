#pragma once

#include "custom-types/shared/macros.hpp"
#include "UnityEngine/MonoBehaviour.hpp"
#include "GlobalNamespace/SaberType.hpp"

DECLARE_CLASS_CODEGEN(Scribble, InputManager, UnityEngine::MonoBehaviour,
    DECLARE_INSTANCE_FIELD(Il2CppString*, inputString);
    DECLARE_INSTANCE_METHOD(void, Start);
    DECLARE_INSTANCE_METHOD(void, Update);

    public:
        GlobalNamespace::SaberType saberType;
        
        UnorderedEventCallback<> buttonPressedEvent;
        UnorderedEventCallback<> buttonReleasedEvent;
    private:
        bool upTriggered = true;

)