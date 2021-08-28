#pragma once

struct config_t {
    bool visibleDuringPlay = false;
    bool useRealGlow = true;
    bool firstTimeLaunch = true;
    bool loadAnimated = true;
    int thumbnailSize = 1024;
};

extern config_t config;

bool LoadConfig();
void SaveConfig();