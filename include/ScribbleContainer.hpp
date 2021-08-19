#pragma once

#include "custom-types/shared/macros.hpp"
#include "UnityEngine/MonoBehaviour.hpp"
#include "UnityEngine/LineRenderer.hpp"
#include "UnityEngine/Coroutine.hpp"

#include "GlobalNamespace/SaberType.hpp"

#include "sombrero/shared/Vector3Utils.hpp"

#include "CustomBrush.hpp"
#include "CustomTypes/LineRenderer.hpp"
#include <vector>

DECLARE_CLASS_CODEGEN(Scribble, LineRendererData, Il2CppObject,
    DECLARE_INSTANCE_FIELD(Scribble::LineRenderer*, lineRenderer);
    CustomBrush* brush;
    LineRendererData* Create(Scribble::LineRenderer* linerenderer, CustomBrush* brush);
)

DECLARE_CLASS_CODEGEN(Scribble, ScribbleContainer, UnityEngine::MonoBehaviour,
    
    DECLARE_INSTANCE_METHOD(void, Start);

    DECLARE_INSTANCE_FIELD(Scribble::LineRenderer*, currentLineRendererLeft);
    DECLARE_INSTANCE_FIELD(Scribble::LineRenderer*, currentLineRendererRight);
    DECLARE_INSTANCE_FIELD(List<LineRendererData*>*, lineRenderers);

    DECLARE_INSTANCE_FIELD(UnityEngine::Coroutine*, animatedLoadRoutine);

    public:
        ScribbleContainer* get_instance();
        static inline bool drawingEnabled = true;
        static inline float lineWidth = 0.001f;

        bool get_IsInAnimation();
        static void Create();
        Scribble::LineRenderer* InitLineRenderer(const CustomBrush& brush, bool disableOnStart = true);

        void UpdateMaterials(float BPM);
        
        void InitPoint(Sombrero::FastVector3 point, GlobalNamespace::SaberType saberType, CustomBrush& brush);
        void AddPoint(Sombrero::FastVector3 point, GlobalNamespace::SaberType saberType);
        std::vector<Sombrero::FastVector3> GetAllPoints();
        void Erase(Sombrero::FastVector3 position, float size);
        
        void Clear();
        void Undo();
        void Delete(int index);
        void Delete(LineRendererData* data);
        void SetLayer(int layer);
        void Show();
        void Hide();

        void CheckLine(GlobalNamespace::SaberType saberType);
    private:
        static ScribbleContainer* instance;
        static constexpr const int minPositionCount = 2;
)