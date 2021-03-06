#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "CustomBrush.hpp"

#include "UnityEngine/Material.hpp"
#include "UnityEngine/Shader.hpp"

#include "CustomTypes/BrushBehaviour.hpp"

namespace Scribble
{
    class Effect;

    class Effects
    {
            static std::vector<Effect*> effects;
        public:
            static const std::vector<Effect*> GetEffects();
            static std::string GetEffectName(int idx);
            static void LoadEffects();
            static Effect* GetEffect(std::string_view name);
    };

    class Effect 
    {
        private:
            std::string name;
            std::string shader;
        public:
            Effect(std::string_view name, std::string_view shader) : name(name), shader(shader) {}; 
            [[nodiscard]] UnityEngine::Shader* get_shader() const;
            [[nodiscard]] std::string get_name() const;
            virtual UnityEngine::Material* CreateMaterial(const CustomBrush& brush);
            friend Effect* Effects::GetEffect(std::string_view name);
            friend UnityEngine::GameObject* BrushBehaviour::CreateBrushMesh();
    };

    class StandardEffect : public Effect
    {
        public:
            StandardEffect(std::string_view name, std::string_view shader) : Effect(name, shader) {};
            UnityEngine::Material* CreateMaterial(const CustomBrush& brush) override;
    };

    class AnimatedEffect : public Effect
    {
        public:
            AnimatedEffect(std::string_view name, std::string_view shader) : Effect(name, shader) {};
            UnityEngine::Material* CreateMaterial(const CustomBrush& brush) override;
    };

    class DotBPM : public Effect
    {
        public:
            DotBPM(std::string_view name, std::string_view shader) : Effect(name, shader) {};
            UnityEngine::Material* CreateMaterial(const CustomBrush& brush) override;
    };

    class Rainbow : public Effect
    {
        public:
            Rainbow(std::string_view name, std::string_view shader) : Effect(name, shader) {};
            UnityEngine::Material* CreateMaterial(const CustomBrush& brush) override;
    };

    class LollyPop : public Effect
    {
        public:
            LollyPop(std::string_view name, std::string_view shader) : Effect(name, shader) {};
            UnityEngine::Material* CreateMaterial(const CustomBrush& brush) override;
    };

    class Outline : public Effect
    {
        public:
            Outline(std::string_view name, std::string_view shader) : Effect(name, shader) {};
            UnityEngine::Material* CreateMaterial(const CustomBrush& brush);
    };
}