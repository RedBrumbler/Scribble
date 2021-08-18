#include "AssetLoader.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"

#include "UnityEngine/AssetBundle.hpp"
#include "UnityEngine/Object.hpp"
#include "static-defines.hpp"

#include "System/Collections/Generic/Dictionary_2.hpp"

template<typename T, typename U>
using Dictionary = System::Collections::Generic::Dictionary_2<T, U>;

using namespace UnityEngine;
namespace Scribble::AssetLoader
{
    SafePtr<AssetBundle> bundle;
    SafePtr<Dictionary<Il2CppString*, Shader*>> shaders;

    void LoadBundle()
    {
        bundle = AssetBundle::LoadFromFile(il2cpp_utils::newcsstr(assetPath));
    }

    Shader* LoadShader(std::string_view name)
    {
        if (!shaders) shaders = Dictionary<Il2CppString*, Shader*>::New_ctor();

        auto key = il2cpp_utils::newcsstr(name);
        if (shaders->ContainsKey(key)) {
            // has key, so shader was already loaded once
            return shaders->get_Item(key);
        }   else {
            if (!bundle) LoadBundle();
            auto shader = Object::Instantiate(bundle->LoadAsset<Shader*>(key));
            Object::DontDestroyOnLoad(shader);
            shaders->Add(key, shader);
            return shader;
        }
    }
}