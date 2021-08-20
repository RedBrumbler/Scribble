#pragma once

#include "CustomBrush.hpp"
#include <optional>

namespace Scribble
{
    class Brushes
    {
        public:
            static inline std::vector<CustomBrush> brushes = {};
            static inline CustomBrush effectBrush;
            static void Load();
            static void Save();
            static void AddDefaultBrushes();
            static std::optional<CustomBrush> GetBrush(std::string_view name);
    };
}