#pragma once

#include "custom-types/shared/macros.hpp"
#include "custom-types/shared/coroutine.hpp"
#include "UnityEngine/MonoBehaviour.hpp"
#include "UnityEngine/LineRenderer.hpp"
#include "UnityEngine/Coroutine.hpp"

#include "GlobalNamespace/SaberType.hpp"

#include "sombrero/shared/Vector3Utils.hpp"

#include "CustomBrush.hpp"
#include "CustomTypes/LineRenderer.hpp"
#include <vector>
#include <string_view>
#include <fstream>

DECLARE_CLASS_CODEGEN(Scribble, LinerendererData, Il2CppObject,
    DECLARE_INSTANCE_FIELD(Scribble::LineRenderer*, lineRenderer);
    CustomBrush brush;

    void Serialize(std::ofstream& writer);
    static void Deserialize(std::ifstream& reader);
    
    static LinerendererData* Create(Scribble::LineRenderer* linerenderer, const CustomBrush& brush);
)

DECLARE_CLASS_CODEGEN(Scribble, ScribbleContainer, UnityEngine::MonoBehaviour,
    
    DECLARE_INSTANCE_METHOD(void, Start);

    DECLARE_INSTANCE_FIELD(Scribble::LineRenderer*, currentLineRendererLeft);
    DECLARE_INSTANCE_FIELD(Scribble::LineRenderer*, currentLineRendererRight);
    DECLARE_INSTANCE_FIELD(List<LinerendererData*>*, lineRenderers);

    DECLARE_INSTANCE_FIELD(UnityEngine::Coroutine*, animatedLoadRoutine);

    public:
        static ScribbleContainer* get_instance();
        static inline bool drawingEnabled = false;
        static inline float lineWidth = 0.001f;

        static void SetBPM(float BPM);
        static void SetOffset(float offset);
        static void SetRealGlow(bool real);

        static void Create();

        bool get_IsInAnimation();
        Scribble::LineRenderer* InitLineRenderer(const CustomBrush& brush, bool disableOnStart = true);

        
        void InitPoint(Sombrero::FastVector3 point, GlobalNamespace::SaberType saberType, const CustomBrush& brush);
        void AddPoint(Sombrero::FastVector3 point, GlobalNamespace::SaberType saberType);
        std::vector<Sombrero::FastVector3> GetAllPoints();
        void Erase(Sombrero::FastVector3 position, float size);
        void Bucket(Sombrero::FastVector3 position, float size, const CustomBrush& brush);
        void StartRuler(Sombrero::FastVector3 position, GlobalNamespace::SaberType type, const CustomBrush& brush);
        void UpdateRuler(Sombrero::FastVector3 position, GlobalNamespace::SaberType type);
        void FinishRuler(Sombrero::FastVector3 position, GlobalNamespace::SaberType type);
        
        void Clear();
        void Undo();
        void Delete(int index);
        void Delete(LinerendererData* data);
        void SetLayer(int layer);
        void Show();
        void Hide();
        
        void Save(std::string_view path, bool clear = false);
        void Load(std::string_view path, bool clear = true, bool animated = true);
        custom_types::Helpers::Coroutine LoadAnimated(std::shared_ptr<std::ifstream> reader, int lineCount);
        static void Reset();

        void CheckLine(GlobalNamespace::SaberType saberType);
    private:
        static ScribbleContainer* instance;
        static constexpr const int minPositionCount = 2;
)