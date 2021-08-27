#include "Brushes.hpp"
#include "logging.hpp"
#include "static-defines.hpp"

#include "beatsaber-hook/shared/rapidjson/include/rapidjson/prettywriter.h"
#include "beatsaber-hook/shared/rapidjson/include/rapidjson/stringbuffer.h"

namespace Scribble
{
    void Brushes::Load()
    {
        if (!fileexists(brushSavePath))
        {
            AddDefaultBrushes();
            Save();
            return;
        }

        std::string json = readfile(brushSavePath);
        rapidjson::Document doc;
        doc.Parse(json.c_str());

        auto arr = doc["brushes"].GetArray();

        for (auto& b : arr)
        {
            // construct in place pog
            brushes.emplace_back(b);
        }
        AddDefaultBrushes();
    }

    void Brushes::Save()
    {
        rapidjson::Document doc;
        doc.SetObject();
        rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
        rapidjson::Value array(rapidjson::kArrayType);
        array.SetArray();

        for (const auto& brush : brushes)
        {
            auto val = brush.ToJson(allocator);
            array.PushBack(val, allocator);
        }

        doc.AddMember("brushes", array, allocator);
        // stringify document
        rapidjson::StringBuffer buffer;
        buffer.Clear();

        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
        doc.Accept(writer);

        // make string out of buffer data
        std::string json(buffer.GetString(), buffer.GetSize());

        // write to file
        writefile(brushSavePath, json);
    }

    void Brushes::AddDefaultBrushes()
    {
        auto primary = GetBrush("Primary");
        auto secondary = GetBrush("Secondary");
        auto william = GetBrush("William");

        INFO("primary brush found: %d, secondary: %d, william: %d", primary.has_value(), secondary.has_value(), william.has_value());
        if (!primary)
            brushes.emplace_back("Primary", Sombrero::FastColor(0.90f, 0.20f, 0.20f, 1.0f), "brush", 0.8f);
        if (!secondary)
            brushes.emplace_back("Secondary", Sombrero::FastColor(0.14f, 0.56f, 0.91f, 1.0f), "brush", 0.8f);
        if (!william)
            brushes.emplace_back("William", Sombrero::FastColor(0.14f, 0.56f, 0.91f, 1.0f), "brush", "rainbow", 0.8f);
    }

    std::optional<std::reference_wrapper<CustomBrush>> Brushes::GetBrush(std::string_view name)
    {
        for (auto& brush : brushes)
            if (!strcmp(brush.name.c_str(), name.data())) return std::make_optional(std::ref(brush));

        return std::nullopt;
    }
}