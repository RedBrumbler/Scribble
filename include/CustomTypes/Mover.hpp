#pragma once

#include "custom-types/shared/macros.hpp"
#include "UnityEngine/MonoBehaviour.hpp"
#include "sombrero/shared/Vector3Utils.hpp"
#include "CustomTypes/BrushBehaviour.hpp"

DECLARE_CLASS_CODEGEN(Scribble, Mover, UnityEngine::MonoBehaviour,
        DECLARE_INSTANCE_FIELD(Scribble::BrushBehaviour*, brushBehaviour);
        DECLARE_INSTANCE_METHOD(void, Awake);
        DECLARE_INSTANCE_METHOD(void, Update);

    public:
        float moverSize = 0.1f;
        
        void Init(Scribble::BrushBehaviour* brushBehaviour);
        void StartMoving();
        void StopMoving();

    private:
        static const constexpr Sombrero::FastColor moverColor = Sombrero::FastColor(0.55f, 0.25f, 0.75f, 1.0f);
        Sombrero::FastVector3 lastMovePosition;
)