//ScriptHook
#include <natives.h>
// #include <types.h> //Already included in globals.h
#include <main.h>
//Custom
#include "script.h"
#include "globals.h"
#include "utils\functions.h"
#include "utils\actions.h"
#include "utils\ini.h"

static void update()
{
	UpdateMenu();

	// Check if player ped exists
	if (!DOES_ENTITY_EXIST(GetPlayerPed()))
		return;

	UpdateSequences();
	return;
}

void ScriptMain()
{
	ReadINI();
	LoadMenuSettings();
	while (true)
	{
		update();
		WAIT(0);
	}
	return;
}