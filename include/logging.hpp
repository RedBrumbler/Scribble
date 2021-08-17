#pragma once
#include "beatsaber-hook/shared/utils/logging.hpp"
#include <string_view>

namespace Scribble
{
    class Logging 
    {
        public:
            static Logger& getLogger();
            static LoggerContextObject& getContextLogger(const char* fun, const char* file, int line);
    };
}
#define INFO(...) ::Scribble::Logging::getContextLogger(__PRETTY_FUNCTION__, __FILE__, __LINE__).info(__VA_ARGS__)
#define ERROR(...) ::Scribble::Logging::getContextLogger(__PRETTY_FUNCTION__, __FILE__, __LINE__).error(__VA_ARGS__)