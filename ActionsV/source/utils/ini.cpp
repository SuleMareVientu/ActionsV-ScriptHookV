#include <SimpleIni.h>
#include "ini.h"
#include "../globals.h"

void ReadINI()
{
	CSimpleIniA ini;
	SI_Error res = ini.LoadFile("ActionsV.ini");

	if (res != SI_OK)
		return;



	return;
}