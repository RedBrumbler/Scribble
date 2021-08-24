#pragma once

#include "UnityEngine/Texture2D.hpp"
#include <string_view>
#include <map>

namespace Scribble::BrushTextures
{
    void LoadAllTextures();
    UnityEngine::Texture2D* GetTexture(std::string_view name);
    std::string GetTextureName(int idx);
    const std::map<std::string, UnityEngine::Texture2D*> GetTextures();

}