#include "CustomTypes/BrushBehaviour.hpp"
#include "CustomTypes/Eraser.hpp"
#include "Effects.hpp"
#include "Brushes.hpp"
#include "GlobalBrushManager.hpp"
#include "ScribbleContainer.hpp"
#include "logging.hpp"

#include "UnityEngine/Transform.hpp"
#include "UnityEngine/Time.hpp"
#include "UnityEngine/PrimitiveType.hpp"
#include "UnityEngine/MeshRenderer.hpp"
#include "UnityEngine/EventSystems/PointerEventData.hpp"
#include "GlobalNamespace/VRController.hpp"

DEFINE_TYPE(Scribble, BrushBehaviour);

using namespace UnityEngine;

namespace Scribble
{
    void BrushBehaviour::Start()
    {
        inputManager = get_gameObject()->AddComponent<InputManager*>();
        inputManager->saberType = saberType;
        inputManager->buttonPressedEvent += {&BrushBehaviour::OnPress, this};
        inputManager->buttonReleasedEvent += {&BrushBehaviour::OnRelease, this};

        if (saberType == GlobalNamespace::SaberType::SaberA)
        {
            currentBrush = Brushes::brushes[0];
            GlobalBrushManager::set_leftBrush(this);
        }
        else
        {
            currentBrush = Brushes::brushes[1];
            GlobalBrushManager::set_rightBrush(this);
            GlobalBrushManager::set_activeBrush(this);
        }

        GetMenuHandle();

        brushMesh = CreateBrushMesh();
        
        auto eraser = get_gameObject()->AddComponent<Eraser*>();
        eraser->Init(this);

        INFO("Brush Initialized");
        set_enabled(false);
    }
    
    void BrushBehaviour::Update()
    {
        if (pressed)
        {
            Sombrero::FastVector3 position = get_transform()->get_position();
            float sqrDistance = position.sqrDistance(lastPoint);
            if (sqrDistance > minDistance)
            {
                ScribbleContainer::get_instance()->AddPoint(position, saberType);
                lastPoint = position;
            }
        }

        if (time < 0.2f)
        {
            time += Time::get_deltaTime();
            return;
        }

        if (eraseMode) return;

        if (CheckForUI())
        {
            ShowBrushMesh(false);
            set_menuHandleActive(true);
            didUpdateMeshLastFrame = false;
        }
        else
        {
            if (!didUpdateMeshLastFrame)
            {
                UpdateBrushMesh();
                didUpdateMeshLastFrame = true;
            }
            ShowBrushMesh(true);
            set_menuHandleActive(false);
        }
    }

    void BrushBehaviour::OnPress()
    {
        GlobalBrushManager::set_activeBrush(this);
        if (CheckForUI()) return;
        if (!ScribbleContainer::get_instance()) return;
        if (!ScribbleContainer::drawingEnabled) return;
        //set_menuHandleActive(false);
        if (eraseMode)
        {
            GetComponent<Eraser*>()->StartErasing();
            return;
        }
        lastPoint = get_transform()->get_position();
        ScribbleContainer::get_instance()->InitPoint(lastPoint, saberType, currentBrush);
        pressed = true;
        //UpdateBrushMesh();
        //ShowBrushMesh(true);
    }
    
    void BrushBehaviour::OnRelease()
    {
        //set_menuHandleActive = true;
        if (eraseMode)
        {
            GetComponent<Eraser*>()->StopErasing();
            return;
        }
        if (!pressed) return;
        pressed = false;
        //ShowBrushMesh(false);
        ScribbleContainer::get_instance()->CheckLine(saberType);
    }

    void BrushBehaviour::GetMenuHandle()
    {
        static auto MenuHandle_cs = il2cpp_utils::newcsstr<il2cpp_utils::CreationType::Manual>("MenuHandle");
        menuHandle = get_transform()->Find(MenuHandle_cs)->get_gameObject();
    }

    void BrushBehaviour::UpdateBrushMesh()
    {
        float size = ScribbleContainer::lineWidth * currentBrush.size;
        brushMesh->get_transform()->set_localScale(Sombrero::FastVector3(size, size, size));
        brushMesh->GetComponent<MeshRenderer*>()->get_material()->set_color(currentBrush.color);
    }

    void BrushBehaviour::ShowBrushMesh(bool show)
    {
        auto mr = pointer->vrController->get_gameObject()->GetComponent<MeshRenderer*>();
        if (mr) mr->set_enabled(!show);
        if (brushMesh) brushMesh->SetActive(show);
    }

    bool BrushBehaviour::CheckForUI()
    {
        auto pointerData = pointer->pointerData;
        return (pointerData ? (bool)pointerData->get_pointerCurrentRaycast().get_gameObject() : false);
    }

    UnityEngine::GameObject* BrushBehaviour::CreateBrushMesh()
    {
        auto go = GameObject::CreatePrimitive(PrimitiveType::Sphere);
        go->get_transform()->SetParent(get_transform(), false);
        go->SetActive(false);
        auto mat = Material::New_ctor((Shader*)Effects::GetEffect("standard")->shader);
        mat->set_color(Sombrero::FastColor::white().Alpha(0.5f));
        go->GetComponent<MeshRenderer*>()->set_material(mat);
        return go;
    }

    void BrushBehaviour::set_menuHandleActive(bool value)
    {
        if (menuHandle) menuHandle->SetActive(value);
    }
}