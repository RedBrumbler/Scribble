#pragma once

#include "UnityEngine/Texture2D.hpp"
#include <string_view>
#include <vector>

namespace Scribble::BrushTextures
{
    void LoadAllTextures();
    UnityEngine::Texture2D* GetTexture(std::string_view name);
    std::string GetTextureName(int idx);
    std::vector<std::pair<std::string, UnityEngine::Texture2D*>> GetTextures();
    void Reset();
}