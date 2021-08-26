#pragma once

struct config_t {
    bool visibleDuringPlay = false;
    bool drawingEnabled = false;
    bool firstTimeLaunch = true;
    int thumbnailSize = 1024;
};

extern config_t config;

bool LoadConfig();
void SaveConfig();