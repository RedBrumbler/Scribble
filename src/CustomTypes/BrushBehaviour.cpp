#include "CustomTypes/BrushBehaviour.hpp"
#include "CustomTypes/Eraser.hpp"
#include "CustomTypes/Bucket.hpp"
#include "CustomTypes/Ruler.hpp"
#include "CustomTypes/Mover.hpp"
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
#include "UnityEngine/EventSystems/UIBehaviour.hpp"
#include "GlobalNamespace/VRController.hpp"

DEFINE_TYPE(Scribble, BrushBehaviour);

using namespace UnityEngine;

namespace Scribble
{
    void BrushBehaviour::Start()
    {
        inputManager = get_gameObject()->AddComponent<InputManager*>();
        inputManager->Init(saberType);
        
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

        auto bucket = get_gameObject()->AddComponent<Bucket*>();
        bucket->Init(this);

        auto ruler = get_gameObject()->AddComponent<Ruler*>();
        ruler->Init(this);

        auto mover = get_gameObject()->AddComponent<Mover*>();
        mover->Init(this);

        mode = BrushMode::Brush;

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

        //if (eraseMode || bucketMode) return;
    }

    void BrushBehaviour::OnPress()
    {
        GlobalBrushManager::set_activeBrush(this);
        if (CheckForUI()) return;
        if (!ScribbleContainer::get_instance()) return;
        if (!ScribbleContainer::drawingEnabled) return;
        set_menuHandleActive(false);

        switch (mode)
        {
            case BrushMode::Brush: {
                break;
            }
            case BrushMode::Erase: {
                GetComponent<Eraser*>()->StartErasing();
                return;
            }
            case BrushMode::Bucket: {
                GetComponent<Bucket*>()->StartBucketing();
                return;
            }
            case BrushMode::Ruler: {
                GetComponent<Ruler*>()->StartRuling();
                return;
            }
            case BrushMode::Move: {
                GetComponent<Mover*>()->StartMoving();
                return;
            }
        }

        lastPoint = get_transform()->get_position();
        ScribbleContainer::get_instance()->InitPoint(lastPoint, saberType, currentBrush);
        pressed = true;
        UpdateBrushMesh();
        ShowBrushMesh(true);
    }
    
    void BrushBehaviour::OnRelease()
    {
        set_menuHandleActive(true);
        switch (mode)
        {
            case BrushMode::Brush: {
                break;
            }
            case BrushMode::Erase: {
                GetComponent<Eraser*>()->StopErasing();
                return;
            }
            case BrushMode::Bucket: {
                GetComponent<Bucket*>()->StopBucketing();
                return;
            }
            case BrushMode::Ruler: {
                GetComponent<Ruler*>()->StopRuling();
                return;
            }
            case BrushMode::Move: {
                GetComponent<Mover*>()->StopMoving();
                return;
            }
        }
        if (!pressed) return;
        pressed = false;
        ShowBrushMesh(false);
        ScribbleContainer::get_instance()->CheckLine(saberType);
    }

    void BrushBehaviour::GetMenuHandle()
    {
        static ConstString MenuHandle_cs("MenuHandle");
        menuHandle = get_transform()->Find(MenuHandle_cs)->get_gameObject();
    }

    void BrushBehaviour::UpdateBrushMesh()
    {
        float size = ScribbleContainer::lineWidth * currentBrush.size;
        brushMesh->get_transform()->set_localScale(Sombrero::FastVector3(size, size, size));
        brushMesh->GetComponent<MeshRenderer*>()->get_material()->set_color(currentBrush.color.Alpha(currentBrush.glow));
    }

    void BrushBehaviour::ShowBrushMesh(bool show)
    {
        auto mr = pointer->dyn__vrController()->get_gameObject()->GetComponent<MeshRenderer*>();
        if (mr) mr->set_enabled(!show);
        if (brushMesh) brushMesh->SetActive(show);
        set_enabled(show);
    }

    bool BrushBehaviour::CheckForUI()
    {
        auto pointerData = pointer->dyn__pointerData();
        if (!pointerData) return false;
        auto go = pointerData->get_pointerCurrentRaycast().get_gameObject();
        if (!go) return false;
        return go->GetComponent<UnityEngine::EventSystems::UIBehaviour*>();
    }

    UnityEngine::GameObject* BrushBehaviour::CreateBrushMesh()
    {
        auto go = GameObject::CreatePrimitive(PrimitiveType::Sphere);
        go->get_transform()->SetParent(get_transform(), false);
        go->SetActive(false);
        auto effect = Effects::GetEffect("standard");
        if (effect)
        {
            UnityEngine::Shader* shader = effect->get_shader();
            if (shader)
            {
                auto mat = Material::New_ctor(shader);
                mat->set_color(Sombrero::FastColor::white().Alpha(0.5f));
                go->GetComponent<MeshRenderer*>()->set_material(mat);
            }
        }
        return go;
    }

    void BrushBehaviour::set_menuHandleActive(bool value)
    {
        if (menuHandle) menuHandle->SetActive(value);
    }
}