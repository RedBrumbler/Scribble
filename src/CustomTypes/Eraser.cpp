#include "CustomTypes/Eraser.hpp"

#include "ScribbleContainer.hpp"
#include "UnityEngine/MeshRenderer.hpp"
#include "UnityEngine/Transform.hpp"
#include "logging.hpp"
DEFINE_TYPE(Scribble, Eraser);

using namespace UnityEngine;
namespace Scribble
{
    void Eraser::Awake()
    {
        set_enabled(false);
        eraserSize = 0.1f;
    }

    void Eraser::Update()
    {
        Sombrero::FastVector3 position = get_transform()->get_position();
        float dist = lastErasePosition.sqrDistance(position);
        if (dist > 0.01f)
        {
            lastErasePosition = position;
            ScribbleContainer::get_instance()->Erase(lastErasePosition, eraserSize);
        }
    }

    void Eraser::Init(Scribble::BrushBehaviour* brushBehaviour)
    {
        this->brushBehaviour = brushBehaviour;
    }

    void Eraser::StartErasing()
    {
        brushBehaviour->brushMesh->get_transform()->set_localScale(Sombrero::FastVector3(eraserSize, eraserSize, eraserSize));
        brushBehaviour->brushMesh->GetComponent<MeshRenderer*>()->get_material()->set_color(eraserColor);
        brushBehaviour->brushMesh->SetActive(true);
        set_enabled(true);
    }

    void Eraser::StopErasing()
    {
        brushBehaviour->brushMesh->SetActive(false);
        set_enabled(false);
    }
}