#include "CustomTypes/BrushBehaviour.hpp"
#include "UnityEngine/PrimitiveType.hpp"

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
    }
    
    void BrushBehaviour::Update()
    {

    }

    void BrushBehaviour::OnPress()
    {

    }
    
    void BrushBehaviour::OnRelease()
    {

    }
}