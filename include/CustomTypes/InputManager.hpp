#pragma once

#include "custom-types/shared/macros.hpp"
#include "UnityEngine/MonoBehaviour.hpp"
#include "GlobalNamespace/SaberType.hpp"

DECLARE_CLASS_CODEGEN(Scribble, InputManager, UnityEngine::MonoBehaviour,
    DECLARE_INSTANCE_FIELD(Il2CppString*, inputString);
    DECLARE_INSTANCE_METHOD(void, Update);
    DECLARE_CTOR(ctor);
    public:
        void Init(GlobalNamespace::SaberType type);
        GlobalNamespace::SaberType saberType;
        
        using OnPressEvent = UnorderedEventCallback<>;
        using OnReleaseEvent = UnorderedEventCallback<>;

        OnPressEvent buttonPressedEvent;
        OnReleaseEvent buttonReleasedEvent;
    private:
        bool upTriggered = true;
)