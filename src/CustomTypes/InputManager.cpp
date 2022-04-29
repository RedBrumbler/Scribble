#include "CustomTypes/InputManager.hpp"
#include "UnityEngine/Input.hpp"

DEFINE_TYPE(Scribble, InputManager);

using namespace UnityEngine;
namespace Scribble
{
    void InputManager::ctor()
    {
        INVOKE_CTOR();
    }

    void InputManager::Init(GlobalNamespace::SaberType type)
    {
        saberType = type;
        inputString = (saberType == GlobalNamespace::SaberType::SaberA) ? "TriggerLeftHand" : "TriggerRightHand";
    }

    void InputManager::Update()
    {
        float triggerValue = Input::GetAxis(inputString);
        if (triggerValue > 0.8f && upTriggered)
        {
            upTriggered = false;
            buttonPressedEvent.invoke();
        }
        else if (triggerValue < 0.8f && !upTriggered)
        {
            upTriggered = true;
            buttonReleasedEvent.invoke();
        }
    }
}