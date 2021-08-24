#include "AssetLoader.hpp"
#include "logging.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"

#include "UnityEngine/AssetBundle.hpp"
#include "UnityEngine/Object.hpp"
#include "UnityEngine/Material.hpp"
#include "UnityEngine/Resources.hpp"
#include "static-defines.hpp"

#include "System/Collections/Generic/Dictionary_2.hpp"
#include <map>

template<typename T, typename U>
using Dictionary = System::Collections::Generic::Dictionary_2<T, U>;

using namespace UnityEngine;
namespace Scribble::AssetLoader
{
    SafePtr<AssetBundle> bundle;
    SafePtr<Dictionary<Il2CppString*, Material*>> shaders;
    //std::map<std::string, UnityEngine::Material*> materials;

    void LoadBundle()
    {
        bundle = AssetBundle::LoadFromFile(il2cpp_utils::newcsstr(assetPath));
    }

    Shader* LoadShader(std::string_view name)
    {
        //if (!shaders) shaders = Dictionary<Il2CppString*, Material*>::New_ctor();
        auto key = il2cpp_utils::newcsstr(name);
        /*
        if (shaders->ContainsKey(key)) {
            // has key, so shader was already loaded once
            return shaders->get_Item(key)->get_shader();
        }   else {*/
            if (!bundle) LoadBundle();
            auto shader = bundle->LoadAsset<Shader*>(key);
            auto mat = Material::New_ctor(shader);
            Object::DontDestroyOnLoad(mat);
            //shaders->Add(key, mat);
            return mat->get_shader();
        //}
    }
}