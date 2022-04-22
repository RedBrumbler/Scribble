#include "Effects.hpp"
#include "BrushTextures.hpp"
#include "AssetLoader.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "logging.hpp"

namespace Scribble
{
    std::vector<Effect*> Effects::effects;

    void Effects::LoadEffects()
    {
        effects.clear();

        StandardEffect* standardEffect = new StandardEffect("standard", "Simple");
        AnimatedEffect* animatedEffect = new AnimatedEffect("animated", "Animated");
        DotBPM* dotBPM = new DotBPM("dotbpm", "DotBPM");
        Rainbow* rainbow = new Rainbow("rainbow", "Rainbow");
        LollyPop* lollypop = new LollyPop("lollypop", "Lollypop");
        Outline* outline = new Outline("outline", "Outline");

        effects.push_back(standardEffect);
        effects.push_back(animatedEffect);
        effects.push_back(dotBPM);
        effects.push_back(rainbow);
        effects.push_back(lollypop);
        effects.push_back(outline);
    }

    const std::vector<Effect*> Effects::GetEffects()
    {
        return effects;
    }
    
    std::string Effects::GetEffectName(int idx)
    {
        if (idx < 0 || idx >= effects.size()) return "standard";
        return effects[idx]->get_name();
    }

    Effect* Effects::GetEffect(std::string_view name)
    {
        for (auto effect : effects)
        {
            if (!strcmp(effect->name.c_str(), name.data())) return effect;
        }
        return nullptr;
    }

    UnityEngine::Shader* Effect::get_shader() const
    {
        return AssetLoader::LoadShader(shader);
    }

    std::string Effect::get_name() const
    {
        return name;
    }

    UnityEngine::Material* Effect::CreateMaterial(const CustomBrush& brush)
    {
        INFO("Creating material %s", name.c_str());
        auto mat = UnityEngine::Material::New_ctor(get_shader());

        mat->SetColor("_Color", brush.color);
        auto tex = BrushTextures::GetTexture(brush.textureName);
        if(tex) mat->SetTexture("_Tex", tex);
        mat->SetVector("_Tiling", brush.get_tiling());
        mat->SetFloat("_Glow", brush.glow);
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
        
        mat->SetFloat("_Speed", 1.0f);
        return mat;
    }

    UnityEngine::Material* DotBPM::CreateMaterial(const CustomBrush& brush)
    {
        auto mat = this->Effect::CreateMaterial(brush);

        mat->SetFloat("_Speed", 0.0f);
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

        mat->SetFloat("_Speed", 1.0f);
        return mat;
    }

    UnityEngine::Material* Outline::CreateMaterial(const CustomBrush& brush)
    {
        auto mat = this->Effect::CreateMaterial(brush);
        return mat;
    }
}