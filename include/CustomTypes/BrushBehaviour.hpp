#pragma once

#include "custom-types/shared/macros.hpp"

#include "UnityEngine/MonoBehaviour.hpp"
#include "UnityEngine/GameObject.hpp"

#include "VRUIControls/VRPointer.hpp"

#include "CustomTypes/InputManager.hpp"
#include "CustomBrush.hpp"

#include "sombrero/shared/Vector3Utils.hpp"

DECLARE_CLASS_CODEGEN(Scribble, BrushBehaviour, UnityEngine::MonoBehaviour,
    DECLARE_INSTANCE_FIELD(Scribble::InputManager*, inputManager);
    DECLARE_INSTANCE_FIELD(VRUIControls::VRPointer*, pointer);

    DECLARE_INSTANCE_FIELD(UnityEngine::GameObject*, brushMesh);
    DECLARE_INSTANCE_FIELD(UnityEngine::GameObject*, menuHandle);
    DECLARE_INSTANCE_METHOD(void, Update);
    DECLARE_INSTANCE_METHOD(void, Start);
    public:
        enum class BrushMode {
            Brush,
            Erase,
            Bucket,
            Ruler,
            Move
        };

        GlobalNamespace::SaberType saberType;
        CustomBrush currentBrush;
        BrushMode mode = BrushMode::Brush;
        void GetMenuHandle();
        UnityEngine::GameObject* CreateBrushMesh();
        void UpdateBrushMesh();
        void ShowBrushMesh(bool show);

    private:

        Sombrero::FastVector3 lastPoint;
        // squared normal min distance -> 0.01 * 0.01 = 0.0001
        static constexpr const float minDistance = 0.0001f;
        bool pressed;
        
        float time;
        bool didUpdateMeshLastFrame;

        void OnPress();
        void OnRelease();
        
        bool CheckForUI();

        void set_menuHandleActive(bool value);
)