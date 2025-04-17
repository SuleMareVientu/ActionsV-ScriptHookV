#pragma once

namespace INI
{
//Input Settings
extern int MenuKey;
extern const char* MenuBinds;

//General Settings
extern bool CentreMenuTitle;
extern bool CentreMenuOptions;
extern bool ShowTooltips;
extern float MenuXOffset;
extern float MenuYOffset;
}

void ReadINI();