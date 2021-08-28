#include "config.hpp"
#include "beatsaber-hook/shared/config/config-utils.hpp"
#include "logging.hpp"

config_t config;

Configuration& get_config()
{
    static Configuration config({ID, VERSION});
    config.Load();
    return config;
}

void SaveConfig()
{
    INFO("Saving Configuration...");
    rapidjson::Document& doc = get_config().config;

    doc.RemoveAllMembers();
    doc.SetObject();
    
    rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

    doc.AddMember("visibleDuringPlay", config.visibleDuringPlay, allocator);
    doc.AddMember("useRealGlow", config.useRealGlow, allocator);
    doc.AddMember("firstTimeLaunch", config.firstTimeLaunch, allocator);
    doc.AddMember("thumbnailSize", config.thumbnailSize, allocator);

    get_config().Write();
    INFO("Saved Configuration!");
}

#define GetBool(identifier) \
auto identifier## _itr = doc.FindMember(#identifier); \
    if (identifier## _itr != doc.MemberEnd()) { \
        config.identifier = identifier## _itr->value.GetBool(); \
    } else { \
        foundEverything = false; \
    }

#define GetInt(identifier) \
auto identifier## _itr = doc.FindMember(#identifier); \
    if (identifier## _itr != doc.MemberEnd()) { \
        config.identifier = identifier## _itr->value.GetInt(); \
    } else { \
        foundEverything = false; \
    }

bool LoadConfig()
{
    INFO("Loading Configuration...");
    bool foundEverything = true;
    rapidjson::Document& doc = get_config().config;

    GetBool(visibleDuringPlay);
    GetBool(useRealGlow);
    GetBool(firstTimeLaunch);
    GetInt(thumbnailSize);

    if (foundEverything) INFO("Loaded Configuration!");
    return foundEverything;
}