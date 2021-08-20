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

    get_config().Write();
    INFO("Saved Configuration!");
}


bool LoadConfig()
{
    INFO("Loading Configuration...");
    bool foundEverything = true;
    rapidjson::Document& doc = get_config().config;

    auto visibleDuringPlay_itr = doc.FindMember("visibleDuringPlay");
    if (visibleDuringPlay_itr != doc.MemberEnd()) {
        config.visibleDuringPlay = visibleDuringPlay_itr->value.GetBool();
    } else {
        foundEverything = false;
    }
    
    if (foundEverything) INFO("Loaded Configuration!");
    return foundEverything;
}