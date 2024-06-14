//ScriptHook
#include <natives.h>
// #include <types.h> //Already included in globals.h
#include <main.h>
//Custom
#include "script.h"
#include "globals.h"
#include "utils\functions.h"
#include "utils\actions.h"

Player player = NULL;
Ped playerPed = NULL;

static void update()
{
	player = PLAYER_ID();
	playerPed = PLAYER_PED_ID();

	// Check if player ped exists and control is on (e.g. not in a cutscene)
	if (!ENTITY::DOES_ENTITY_EXIST(playerPed) || !IS_PLAYER_CONTROL_ON(player))
		return;

	UpdateSequences();
	return;
}

void ScriptMain()
{
	while (true)
	{
		update();
		WAIT(0);
	}
	return;
}