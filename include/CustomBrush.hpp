#pragma once

#include <string>
#include "sombrero/shared/ColorUtils.hpp"

namespace Scribble
{
    class CustomBrush
    {
        public:
            float tiling;
            float glow;
            std::string textureName;
            Sombrero::FastColor color;
    };
}