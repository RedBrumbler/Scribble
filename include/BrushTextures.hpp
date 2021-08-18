#pragma once

#include "UnityEngine/Texture2D.hpp"
#include <string_view>

namespace Scribble::BrushTextures
{
    UnityEngine::Texture2D* GetTexture(std::string_view name);
}