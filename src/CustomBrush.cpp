#include "CustomBrush.hpp"
#include "Effects.hpp"

using namespace UnityEngine;
namespace Scribble
{
    CustomBrush::CustomBrush(const rapidjson::Value& val)
    {
        name = val["name"].GetString();
        auto color_itr = val.FindMember("color");
        color = Sombrero::FastColor(color_itr->value["r"].GetDouble(), color_itr->value["g"].GetDouble(), color_itr->value["b"].GetDouble(), 1.0f);
        textureName = val["textureName"].GetString();
        effectName = val["effectName"].GetString();
        size = val["size"].GetDouble();
        glow = val["glow"].GetDouble();
        textureMode = (TextureMode)val["textureMode"].GetInt();
        auto tiling_itr = val.FindMember("tiling");
        tiling = Sombrero::FastVector2(tiling_itr->value["x"].GetDouble(), tiling_itr->value["y"].GetDouble());
    }

    CustomBrush::CustomBrush() : CustomBrush("unnamed", Sombrero::FastColor::white(), "brush", "standard", 20, 1.0f, TextureMode::Stretch, Sombrero::FastVector2::one()) {};
    CustomBrush::CustomBrush(std::string_view name) : CustomBrush(name, Sombrero::FastColor::white(), "brush", "standard", 20, 1.0f, TextureMode::Stretch, Sombrero::FastVector2::one()) {};
    CustomBrush::CustomBrush(std::string_view name, Sombrero::FastColor color) : CustomBrush(name, color, "brush", "standard", 20, 1.0f, TextureMode::Stretch, Sombrero::FastVector2::one()) {};
    CustomBrush::CustomBrush(std::string_view name, Sombrero::FastColor color, float glow) : CustomBrush(name, color, "brush", "standard", 20, glow, TextureMode::Stretch, Sombrero::FastVector2::one()) {};
    CustomBrush::CustomBrush(std::string_view name, Sombrero::FastColor color, std::string_view textureName, float glow) : CustomBrush(name, color, textureName, "standard", 20, glow, TextureMode::Stretch, Sombrero::FastVector2::one()) {};
    CustomBrush::CustomBrush(std::string_view name, Sombrero::FastColor color, std::string_view textureName, std::string_view effectName, float glow) : CustomBrush(name, color, textureName, effectName, 20, glow, TextureMode::Stretch, Sombrero::FastVector2::one()) {};
    CustomBrush::CustomBrush(std::string_view name, Sombrero::FastColor color, std::string_view textureName, std::string_view effectName, int size, float glow) : CustomBrush(name, color, textureName, effectName, size, glow, TextureMode::Stretch, Sombrero::FastVector2::one()) {};
    CustomBrush::CustomBrush(std::string_view name, Sombrero::FastColor color, std::string_view textureName, std::string_view effectName, int size, float glow, TextureMode textureMode) : CustomBrush(name, color, textureName, effectName, size, glow, textureMode, Sombrero::FastVector2::one()) {};

    CustomBrush::CustomBrush(std::string_view name, Sombrero::FastColor color, std::string_view textureName, std::string_view effectName, int size, float glow, TextureMode textureMode, Sombrero::FastVector2 tiling)
    {
        this->name = name;
        this->color = color;
        this->textureName = textureName;
        this->effectName = effectName;
        this->size = size;
        this->glow = glow;
        this->textureMode = textureMode;
        this->tiling = tiling;
    }

    Material* CustomBrush::CreateMaterial() const
    {
        return CreateMaterial(*this);
    }

    Material* CustomBrush::CreateMaterial(const CustomBrush& brush)
    {
        auto effect = Effects::GetEffect(brush.effectName);
        auto mat = effect->CreateMaterial(brush);
        return mat;
    }
    
    rapidjson::Value CustomBrush::ToJson(rapidjson::Document::AllocatorType& allocator) const
    {
        return ToJson(*this, allocator);
    }

    rapidjson::Value CustomBrush::ToJson(const CustomBrush& brush, rapidjson::Document::AllocatorType& allocator)
    {
        rapidjson::Value val;
        val.SetObject();

        val.AddMember("name", rapidjson::Value(brush.name.c_str(), brush.name.size(), allocator), allocator);
        rapidjson::Value colorVal;
        colorVal.SetObject();

        colorVal.AddMember("r", brush.color.r, allocator);
        colorVal.AddMember("g", brush.color.g, allocator);
        colorVal.AddMember("b", brush.color.b, allocator);
        val.AddMember("color", colorVal, allocator);
        
        val.AddMember("textureName", rapidjson::Value(brush.textureName.c_str(), brush.textureName.size(), allocator), allocator);
        val.AddMember("effectName", rapidjson::Value(brush.effectName.c_str(), brush.effectName.size(), allocator), allocator);
        val.AddMember("size", brush.size, allocator);
        val.AddMember("glow", brush.glow, allocator);
        val.AddMember("textureMode", (int)brush.textureMode, allocator);

        rapidjson::Value tilingVal;
        tilingVal.SetObject();
        
        tilingVal.AddMember("x", brush.tiling.x, allocator);
        tilingVal.AddMember("y", brush.tiling.y, allocator);
        val.AddMember("tiling", tilingVal, allocator);

        return val;
    }

    // im a liar
    void CustomBrush::copy(const CustomBrush& brush) const
    {
        *const_cast<CustomBrush*>(this) = brush;
    }
}