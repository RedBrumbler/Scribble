#pragma once

#include "custom-types/shared/macros.hpp"
#include "UnityEngine/MonoBehaviour.hpp"

DECLARE_CLASS_CODEGEN(Scribble, ScribbleContainer, UnityEngine::MonoBehaviour,
        
    public:
        ScribbleContainer* get_instance();
    private:
        SafePtr<ScribbleContainer> instance;
)