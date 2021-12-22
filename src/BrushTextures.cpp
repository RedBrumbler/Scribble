#include "BrushTextures.hpp"
#include "Utils/FileUtils.hpp"

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

    void LoadAllTextures()
    {
        std::vector<std::string> textures;
        FileUtils::GetFilesInFolderPath("png", brushTexturePath, textures);
        
        for (auto t : textures)
        {
            GetTexture(FileUtils::GetFileName(t, true));
        }
    }

    Texture2D* GetTexture(std::string_view name)
    {
        if (!textures || !((Dictionary<Il2CppString*, UnityEngine::Texture2D*>*)textures)) textures = Dictionary<Il2CppString*, UnityEngine::Texture2D*>::New_ctor();
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

    std::string GetTextureName(int idx)
    {
        if (!textures || idx < 0 || idx >= textures->get_Count()) return "brush";
        return to_utf8(csstrtostr(textures->entries[idx].key));
    }

    const std::vector<std::pair<std::string, UnityEngine::Texture2D*>> GetTextures()
    {
        std::vector<std::pair<std::string, UnityEngine::Texture2D*>> tex;

        int count = textures->get_Count();
        for (int i = 0; i < count; i++)
        {
            std::string key = to_utf8(csstrtostr(textures->entries[i].key));
            tex.emplace_back(key, textures->entries[i].value);
        }
        return tex;
    }

    void Reset()
    {
        textures.emplace(nullptr);
    }
}