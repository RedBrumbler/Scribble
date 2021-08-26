#pragma once
#include "beatsaber-hook/shared/rapidjson/include/rapidjson/document.h"

#include <string_view>
#include "sombrero/shared/ColorUtils.hpp"
#include "sombrero/shared/Vector2Utils.hpp"

#include "UnityEngine/Material.hpp"
#include "UnityEngine/Vector4.hpp"

#include <fstream>

namespace Scribble
{
    class CustomBrush
    {
        public:
            enum TextureMode {
                Stretch,
                Tile
            };

            CustomBrush(const rapidjson::Value& val);

            CustomBrush();
            CustomBrush(std::string_view name);
            CustomBrush(std::string_view name, Sombrero::FastColor color);
            CustomBrush(std::string_view name, Sombrero::FastColor color, float glow);
            CustomBrush(std::string_view name, Sombrero::FastColor color, std::string_view textureName, float glow);
            CustomBrush(std::string_view name, Sombrero::FastColor color, std::string_view textureName, std::string_view effectName, float glow);
            CustomBrush(std::string_view name, Sombrero::FastColor color, std::string_view textureName, std::string_view effectName, int size, float glow);
            CustomBrush(std::string_view name, Sombrero::FastColor color, std::string_view textureName, std::string_view effectName, int size, float glow, TextureMode textureMode);
            CustomBrush(std::string_view name, Sombrero::FastColor color, std::string_view textureName, std::string_view effectName, int size, float glow, TextureMode textureMode, Sombrero::FastVector2 tiling);

            std::string name = "unnamed";
            int index = 0;
            Sombrero::FastColor color = Sombrero::FastColor::white();
            std::string textureName = "standard";
            std::string effectName = "standard";
            int size = 20;
            float glow = 1.0f;
            TextureMode textureMode = TextureMode::Stretch;
            Sombrero::FastVector2 tiling = Sombrero::FastVector2::one();
            
            UnityEngine::Material* CreateMaterial() const;
            static UnityEngine::Material* CreateMaterial(const CustomBrush& brush);

            UnityEngine::Vector4 get_tiling() const { return UnityEngine::Vector4(tiling.x, tiling.y, 0.0f, 0.0f); };
            
            rapidjson::Value ToJson(rapidjson::Document::AllocatorType& allocator) const;
            static rapidjson::Value ToJson(const CustomBrush& brush, rapidjson::Document::AllocatorType& allocator);

            // this method is const I promise
            void copy(const CustomBrush& brush) const;

            void Serialize(std::ofstream& writer);
            static CustomBrush Deserialize(std::ifstream& reader);
    };
}