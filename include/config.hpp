#pragma once

struct config_t {
    bool visibleDuringPlay = false;
    bool drawingEnabled = false;
    bool firstTimeLaunch = true;
};

extern config_t config;

bool LoadConfig();
void SaveConfig();