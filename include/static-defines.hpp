#pragma once

#define DATAPATH "/sdcard/ModData/com.beatgames.beatsaber/Mods/Scribble/"

constexpr const char* brushTexturePath = DATAPATH "BrushTextures";
constexpr const char* assetPath = DATAPATH "scribbleassets";
constexpr const char* brushSavePath = DATAPATH "brushes.json";

constexpr const int availableEffectCount = 6;
constexpr const char* availableEffectNames[availableEffectCount] = {
    "Animated",
    "DotBPM",
    "Lollypop",
    "Outline",
    "Rainbow",
    "Simple"
};