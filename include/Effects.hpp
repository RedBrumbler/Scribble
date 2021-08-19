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
            static void LoadEffects();
            static Effect* GetEffect(std::string_view name);
    };

    class Effect 
    {
        private:
            std::string name;
             SafePtr<UnityEngine::Shader> shader;
        public:
            Effect(std::string_view name, UnityEngine::Shader* shader) : name(name), shader(shader) {}; 
            virtual UnityEngine::Material* CreateMaterial(const CustomBrush& brush);
            friend Effect* Effects::GetEffect(std::string_view name);
            friend UnityEngine::GameObject* BrushBehaviour::CreateBrushMesh();
    };

    class StandardEffect : public Effect
    {
        public:
            StandardEffect(std::string_view name, UnityEngine::Shader* shader) : Effect(name, shader) {};
            UnityEngine::Material* CreateMaterial(const CustomBrush& brush);
    };

    class AnimatedEffect : public Effect
    {
        public:
            AnimatedEffect(std::string_view name, UnityEngine::Shader* shader) : Effect(name, shader) {};
            UnityEngine::Material* CreateMaterial(const CustomBrush& brush);
    };

    class DotBPM : public Effect
    {
        public:
            DotBPM(std::string_view name, UnityEngine::Shader* shader) : Effect(name, shader) {};
            UnityEngine::Material* CreateMaterial(const CustomBrush& brush);
    };

    class Rainbow : public Effect
    {
        public:
            Rainbow(std::string_view name, UnityEngine::Shader* shader) : Effect(name, shader) {};
            UnityEngine::Material* CreateMaterial(const CustomBrush& brush);
    };

    class LollyPop : public Effect
    {
        public:
            LollyPop(std::string_view name, UnityEngine::Shader* shader) : Effect(name, shader) {};
            UnityEngine::Material* CreateMaterial(const CustomBrush& brush);
    };

    class Outline : public Effect
    {
        public:
            Outline(std::string_view name, UnityEngine::Shader* shader) : Effect(name, shader) {};
            UnityEngine::Material* CreateMaterial(const CustomBrush& brush);
    };
}