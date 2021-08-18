#pragma once

#include "UnityEngine/Shader.hpp"
#include <string_view>

namespace Scribble::AssetLoader
{
    void LoadBundle();
    UnityEngine::Shader* LoadShader(std::string_view name);
}