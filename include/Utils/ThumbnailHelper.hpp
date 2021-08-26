#pragma once

#include "UnityEngine/Camera.hpp"
#include "UnityEngine/Texture2D.hpp"
#include "sombrero/shared/Vector3Utils.hpp"

#include <fstream>

namespace Scribble::ThumbnailHelper
{
    UnityEngine::Texture2D* GetThumbnail(UnityEngine::Camera* camera, int width, int height);
    void WriteThumbnail(std::ofstream& writer, UnityEngine::Camera* camera, int width, int height);
    UnityEngine::Camera* CreateCamera(Sombrero::FastVector3&& position, Sombrero::FastVector3&& rotation);
    bool CheckPngData(std::ifstream& reader, long& size, bool skip);
    UnityEngine::Texture2D* ReadPNG(std::ifstream& reader, const long& size);
}