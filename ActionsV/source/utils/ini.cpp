#include "ini.h"
#include <SimpleIni.h>
#include "functions.h"
#include "../globals.h"

static constexpr char* inputGroup = "Input";
static constexpr char* optionsGroup = "Options";

namespace INI
{
//Input Settings
int MenuKey = 0x79;
const char* MenuBinds = "PAD_LEFT, LB";

//General Settings
bool CentreMenuTitle = true;
bool CentreMenuOptions = false;
bool ShowTooltips = true;
float MenuXOffset = 0.00f;
float MenuYOffset = 0.00f;
}
using namespace INI;

static CSimpleIniA ini;
void ReadINI()
{
	SI_Error res = ini.LoadFile("ActionsV.ini");

	if (res != SI_OK)
		return;

	//////////////////////////////////////Input//////////////////////////////////////////
	char* DefaultMenuKeyStr = "F10"; std::string tmpStrArr[1]{};
	SplitString(const_cast<char*>(ini.GetValue(inputGroup, "MenuKey", DefaultMenuKeyStr)), tmpStrArr, 1);
	MenuKey = GetVKFromString(tmpStrArr[0]);
	MenuBinds = ini.GetValue(inputGroup, "MenuBinds", MenuBinds);

	//////////////////////////////////////Options////////////////////////////////////////
	CentreMenuTitle = ini.GetBoolValue(optionsGroup, "CentreMenuTitle", CentreMenuTitle);
	CentreMenuOptions = ini.GetBoolValue(optionsGroup, "CentreMenuOptions", CentreMenuOptions);
	ShowTooltips = ini.GetBoolValue(optionsGroup, "ShowTooltips", ShowTooltips);
	MenuXOffset = static_cast<float>(ini.GetDoubleValue(optionsGroup, "MenuXOffset", MenuXOffset));
	MenuYOffset = static_cast<float>(ini.GetDoubleValue(optionsGroup, "MenuYOffset", MenuYOffset));
	return;
}