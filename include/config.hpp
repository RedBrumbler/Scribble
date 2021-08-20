#pragma once

struct config_t {
    bool visibleDuringPlay;
};

extern config_t config;

bool LoadConfig();
void SaveConfig();