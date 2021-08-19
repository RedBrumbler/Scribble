#include "Brushes.hpp"
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
    }

    void Brushes::Save()
    {
        rapidjson::Document doc;
        doc.SetObject();
        rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
        rapidjson::Value array(rapidjson::kArrayType);

        for (const auto& brush : brushes)
        {
            array.PushBack(brush.ToJson(allocator), allocator);
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
        if (!GetBrush("Primary"))
            brushes.push_back(CustomBrush("Primary", Sombrero::FastColor(0.90f, 0.20f, 0.20f, 1.0f), "brush", 0.8f));
        if (!GetBrush("Secondary"))
            brushes.push_back(CustomBrush("Secondary", Sombrero::FastColor(0.14f, 0.56f, 0.91f, 1.0f), "brush", 0.8f));
    }

    std::optional<CustomBrush> Brushes::GetBrush(std::string_view name)
    {
        for (auto& brush : brushes)
            if (!strcmp(brush.name.c_str(), name.data())) return brush;

        return std::nullopt;
    }
}