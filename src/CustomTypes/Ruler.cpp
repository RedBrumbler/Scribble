#include "CustomTypes/Ruler.hpp"

#include "ScribbleContainer.hpp"
#include "UnityEngine/MeshRenderer.hpp"
#include "UnityEngine/Transform.hpp"
#include "logging.hpp"
DEFINE_TYPE(Scribble, Ruler);

using namespace UnityEngine;
namespace Scribble
{
    void Ruler::Awake()
    {
        set_enabled(false);
        rulerSize = 0.1f;
    }

    void Ruler::Update()
    {
        if (pressed) ScribbleContainer::get_instance()->UpdateRuler(get_transform()->get_position(), brushBehaviour->saberType);
    }

    void Ruler::Init(Scribble::BrushBehaviour* brushBehaviour)
    {
        this->brushBehaviour = brushBehaviour;
    }

    void Ruler::StartRuling()
    {
        INFO("Start Ruling");
        pressed = true;
        brushBehaviour->brushMesh->get_transform()->set_localScale(Sombrero::FastVector3(rulerSize, rulerSize, rulerSize));
        brushBehaviour->brushMesh->GetComponent<MeshRenderer*>()->get_material()->set_color(rulerColor);
        brushBehaviour->brushMesh->SetActive(true);
        ScribbleContainer::get_instance()->StartRuler(get_transform()->get_position(), brushBehaviour->saberType, brushBehaviour->currentBrush);
        set_enabled(true);
    }

    void Ruler::StopRuling()
    {
        if (!pressed) return;
        pressed = false;
        INFO("Stop Ruling");
        brushBehaviour->brushMesh->SetActive(false);
        ScribbleContainer::get_instance()->FinishRuler(get_transform()->get_position(), brushBehaviour->saberType);
        set_enabled(false);
    }
}