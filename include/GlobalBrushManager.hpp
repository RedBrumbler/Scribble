#pragma once
#include "beatsaber-hook/shared/utils/typedefs-wrappers.hpp"

#include "CustomBrush.hpp"
#include "CustomTypes/BrushBehaviour.hpp"

namespace Scribble::GlobalBrushManager
{
    using OnActiveBrushChangedEvent = UnorderedEventCallback<BrushBehaviour*>;
    
    BrushBehaviour* get_leftBrush();
    void set_leftBrush(BrushBehaviour* value);

    BrushBehaviour* get_rightBrush();
    void set_rightBrush(BrushBehaviour* value);

    BrushBehaviour* get_activeBrush();
    void set_activeBrush(BrushBehaviour* value);

    OnActiveBrushChangedEvent& OnActiveBrushChanged();
}