#include "BrushTextures.hpp"
#include "logging.hpp"
#include "static-defines.hpp"
#include <fstream>

#include "UnityEngine/TextureWrapMode.hpp"
#include "UnityEngine/ImageConversion.hpp"
#include "UnityEngine/TextureFormat.hpp"
#include "UnityEngine/Object.hpp"

#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp" 

#include "System/Collections/Generic/Dictionary_2.hpp"

template<typename T, typename U>
using Dictionary = System::Collections::Generic::Dictionary_2<T, U>;

using namespace UnityEngine;
namespace Scribble::BrushTextures
{
    SafePtr<Dictionary<Il2CppString*, UnityEngine::Texture2D*>> textures;

    Texture2D* GetTexture(std::string_view name)
    {
        if (!textures) textures = Dictionary<Il2CppString*, UnityEngine::Texture2D*>::New_ctor();
        auto key = il2cpp_utils::newcsstr(name);
        Texture2D* tex = nullptr;
        if (textures->ContainsKey(key)) {
            // if found, get the value
            return textures->get_Item(key);
        } else {
            // if not found, load the texture from file, even if the texture doesnt exist this will work
            std::string path = string_format("%s/%s.png", brushTexturePath, name.data());
            std::ifstream instream(path, std::ios::in | std::ios::binary);
            std::vector<uint8_t> data((std::istreambuf_iterator<char>(instream)), std::istreambuf_iterator<char>());
            Array<uint8_t>* bytes = il2cpp_utils::vectorToArray(data);
            tex = Texture2D::New_ctor(0, 0, UnityEngine::TextureFormat::RGBA32, false, false);
            ImageConversion::LoadImage(tex, bytes, false);
            Object::DontDestroyOnLoad(tex);
            tex->set_wrapMode(TextureWrapMode::Repeat);
            textures->Add(key, tex);
            return tex;
        }
    }
}