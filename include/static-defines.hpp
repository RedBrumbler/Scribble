#pragma once

#define DATAPATH(path) "/sdcard/ModData/com.beatgames.beatsaber/Mods/" MOD_ID "/" path

constexpr const int imageVersion = 1;
constexpr const char* brushTexturePath = DATAPATH("BrushTextures");
constexpr const char* assetPath = DATAPATH("scribbleassets";
constexpr const char* brushSavePath = DATAPATH("brushes.json";
constexpr const char* drawingPath = DATAPATH("Drawings";
constexpr const char* modelsPath = DATAPATH("Models");

constexpr const int availableEffectCount = 6;
constexpr const char* availableEffectNames[availableEffectCount] = {
    "Animated",
    "DotBPM",
    "Lollypop",
    "Outline",
    "Rainbow",
    "Simple"};