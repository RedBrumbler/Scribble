#pragma once

#include "custom-types/shared/macros.hpp"
#include "UnityEngine/MonoBehaviour.hpp"
#include "sombrero/shared/Vector3Utils.hpp"
#include "CustomTypes/BrushBehaviour.hpp"

DECLARE_CLASS_CODEGEN(Scribble, Ruler, UnityEngine::MonoBehaviour,
        DECLARE_INSTANCE_FIELD(Scribble::BrushBehaviour*, brushBehaviour);
        DECLARE_INSTANCE_METHOD(void, Awake);
        DECLARE_INSTANCE_METHOD(void, Update);

    public:
        float rulerSize = 0.1f;
        bool pressed;
        void Init(Scribble::BrushBehaviour* brushBehaviour);
        void StartRuling();
        void StopRuling();

    private:
        static const constexpr Sombrero::FastColor rulerColor = Sombrero::FastColor(0.01f, 0.98f, 0.15f, 1.0f);
        Sombrero::FastVector3 lastRulerPosition;
)