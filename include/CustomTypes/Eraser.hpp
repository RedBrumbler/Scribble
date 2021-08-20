#pragma once

#include "custom-types/shared/macros.hpp"
#include "UnityEngine/MonoBehaviour.hpp"
#include "sombrero/shared/Vector3Utils.hpp"
#include "CustomTypes/BrushBehaviour.hpp"

DECLARE_CLASS_CODEGEN(Scribble, Eraser, UnityEngine::MonoBehaviour,
        DECLARE_INSTANCE_FIELD(Scribble::BrushBehaviour*, brushBehaviour);
        DECLARE_INSTANCE_METHOD(void, Awake);
        DECLARE_INSTANCE_METHOD(void, Update);

    public:
        float eraserSize = 0.1f;
        
        void Init(Scribble::BrushBehaviour* brushBehaviour);
        void StartErasing();
        void StopErasing();

    private:
        static const constexpr Sombrero::FastColor eraserColor = Sombrero::FastColor(0.98f, 0.15f, 0.01f, 1.0f);
        Sombrero::FastVector3 lastErasePosition;
)