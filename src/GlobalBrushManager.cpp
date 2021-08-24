#include "GlobalBrushManager.hpp"

namespace Scribble::GlobalBrushManager
{
    OnActiveBrushChangedEvent onActiveBrushChangedEvent;
    BrushBehaviour* leftBrush = nullptr;
    BrushBehaviour* rightBrush = nullptr;
    BrushBehaviour* activeBrush = nullptr;
    
    BrushBehaviour* get_leftBrush()
    {
        return leftBrush;
    }

    void set_leftBrush(BrushBehaviour* value)
    {
        leftBrush = value;
    }

    BrushBehaviour* get_rightBrush()
    {
        return rightBrush;
    }

    void set_rightBrush(BrushBehaviour* value)
    {
        rightBrush = value;
    }

    BrushBehaviour* get_activeBrush()
    {
        return activeBrush;
    }

    void set_activeBrush(BrushBehaviour* value)
    {
        if (value == activeBrush) return;
        activeBrush = value;
        onActiveBrushChangedEvent.invoke(value);
    }

    OnActiveBrushChangedEvent& OnActiveBrushChanged()
    {
        return onActiveBrushChangedEvent;
    }
}