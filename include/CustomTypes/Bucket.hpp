#pragma once

#include "custom-types/shared/macros.hpp"
#include "UnityEngine/MonoBehaviour.hpp"
#include "sombrero/shared/Vector3Utils.hpp"
#include "CustomTypes/BrushBehaviour.hpp"

DECLARE_CLASS_CODEGEN(Scribble, Bucket, UnityEngine::MonoBehaviour,
        DECLARE_INSTANCE_FIELD(Scribble::BrushBehaviour*, brushBehaviour);
        DECLARE_INSTANCE_METHOD(void, Awake);
        DECLARE_INSTANCE_METHOD(void, Update);

    public:
        float bucketSize = 0.1f;
        
        void Init(Scribble::BrushBehaviour* brushBehaviour);
        void StartBucketing();
        void StopBucketing();

    private:
        static const constexpr Sombrero::FastColor bucketColor = Sombrero::FastColor(0.5f, 0.7f, 0.98f, 1.0f);
        Sombrero::FastVector3 lastBucketPosition;
)