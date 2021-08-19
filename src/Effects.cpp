#include "Effects.hpp"
#include "BrushTextures.hpp"
#include "AssetLoader.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "logging.hpp"

static Il2CppString* _Color = nullptr;
static Il2CppString* _Color2 = nullptr;
static Il2CppString* _Speed = nullptr;
static Il2CppString* _Tex = nullptr;
static Il2CppString* _Tiling = nullptr;
static Il2CppString* _Glow = nullptr;

#define EnsureString(identifier) \
if (!identifier) identifier = il2cpp_utils::newcsstr<il2cpp_utils::CreationType::Manual>(#identifier)
namespace Scribble
{
    std::vector<Effect*> Effects::effects;

    void Effects::LoadEffects()
    {
            effects.clear();

            StandardEffect* standardEffect = new StandardEffect("standard", AssetLoader::LoadShader("Simple"));
            AnimatedEffect* animatedEffect = new AnimatedEffect("animated", AssetLoader::LoadShader("Animated"));
            DotBPM* dotBPM = new DotBPM("dotbpm", AssetLoader::LoadShader("DotBPM"));
            Rainbow* rainbow = new Rainbow("rainbow", AssetLoader::LoadShader("Rainbow"));
            LollyPop* lollypop = new LollyPop("lollypop", AssetLoader::LoadShader("Lollypop"));
            Outline* outline = new Outline("outline", AssetLoader::LoadShader("Outline"));

            effects.push_back(standardEffect);
            effects.push_back(animatedEffect);
            effects.push_back(dotBPM);
            effects.push_back(rainbow);
            effects.push_back(lollypop);
            effects.push_back(outline);
    }

    Effect* Effects::GetEffect(std::string_view name)
    {
        for (auto effect : effects)
        {
            if (effect->name == name) return effect;
        }
        return nullptr;
    }

    UnityEngine::Material* Effect::CreateMaterial(const CustomBrush& brush)
    {
        if (!shader) return nullptr;
        INFO("Creating material %s", name.c_str());
        auto mat = UnityEngine::Material::New_ctor((UnityEngine::Shader*)shader);
        
        EnsureString(_Color);
        EnsureString(_Tex);
        EnsureString(_Tiling);
        EnsureString(_Glow);

        mat->SetColor(_Color, brush.color);
        auto tex = BrushTextures::GetTexture(brush.textureName);
        if(tex) mat->SetTexture(_Tex, tex);
        mat->SetVector(_Tiling, brush.get_tiling());
        mat->SetFloat(_Glow, brush.glow);
        return mat;
    }

    UnityEngine::Material* StandardEffect::CreateMaterial(const CustomBrush& brush)
    {
        auto mat = this->Effect::CreateMaterial(brush);
        return mat;
    }

    UnityEngine::Material* AnimatedEffect::CreateMaterial(const CustomBrush& brush)
    {
        auto mat = this->Effect::CreateMaterial(brush);

        EnsureString(_Speed);
        
        mat->SetFloat(_Speed, 1.0f);
        return mat;
    }

    UnityEngine::Material* DotBPM::CreateMaterial(const CustomBrush& brush)
    {
        auto mat = this->Effect::CreateMaterial(brush);
        
        return mat;
    }
    
    UnityEngine::Material* Rainbow::CreateMaterial(const CustomBrush& brush)
    {
        auto mat = this->Effect::CreateMaterial(brush);
        return mat;
    }

    UnityEngine::Material* LollyPop::CreateMaterial(const CustomBrush& brush)
    {
        auto mat = this->Effect::CreateMaterial(brush);
        
        EnsureString(_Speed);

        mat->SetFloat(_Speed, 1.0f);
        return mat;
    }

    UnityEngine::Material* Outline::CreateMaterial(const CustomBrush& brush)
    {
        auto mat = this->Effect::CreateMaterial(brush);
        return mat;
    }
}