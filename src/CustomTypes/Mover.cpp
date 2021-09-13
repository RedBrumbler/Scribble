#include "CustomTypes/Mover.hpp"

#include "ScribbleContainer.hpp"
#include "UnityEngine/MeshRenderer.hpp"
#include "UnityEngine/Transform.hpp"
#include "logging.hpp"
DEFINE_TYPE(Scribble, Mover);

using namespace UnityEngine;
namespace Scribble
{
    void Mover::Awake()
    {
        set_enabled(false);
        moverSize = 0.1f;
    }

    void Mover::Update()
    {
        Sombrero::FastVector3 position = get_transform()->get_position();
        ScribbleContainer::get_instance()->Move(position - lastMovePosition);
        lastMovePosition = position;
    }

    void Mover::Init(Scribble::BrushBehaviour* brushBehaviour)
    {
        this->brushBehaviour = brushBehaviour;
    }

    void Mover::StartMoving()
    {
        lastMovePosition = get_transform()->get_position();
        brushBehaviour->brushMesh->get_transform()->set_localScale(Sombrero::FastVector3(moverSize, moverSize, moverSize));
        brushBehaviour->brushMesh->GetComponent<MeshRenderer*>()->get_material()->set_color(moverColor);
        brushBehaviour->brushMesh->SetActive(true);
        set_enabled(true);
    }

    void Mover::StopMoving()
    {
        brushBehaviour->brushMesh->SetActive(false);
        set_enabled(false);
    }
}