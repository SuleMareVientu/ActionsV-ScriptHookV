//ScriptHook
#include <natives.h>
// #include <types.h> //Already included in globals.h
//Custom
#include "functions.h"
#include "..\globals.h"

//Custom implementation of TIMERA and TIMERB natives
void Timer::Set(int value)
{
	gameTimer = GET_GAME_TIMER() + value;
	return;
}

int Timer::Get()
{
	return (GET_GAME_TIMER() - gameTimer);
}

void EnablePedConfigFlag(Ped ped, int flag)
{
	if (!PED::GET_PED_CONFIG_FLAG(ped, flag, false))
		PED::SET_PED_CONFIG_FLAG(ped, flag, true);
	return;
}

void DisablePedConfigFlag(Ped ped, int flag)
{
	if (PED::GET_PED_CONFIG_FLAG(ped, flag, false))
		PED::SET_PED_CONFIG_FLAG(ped, flag, false);
	return;
}

void EnablePedResetFlag(Ped ped, int flag)
{
	if (!PED::GET_PED_RESET_FLAG(ped, flag))
		PED::SET_PED_RESET_FLAG(ped, flag, true);
	return;
}

void DisablePedResetFlag(Ped ped, int flag)
{
	if (PED::GET_PED_RESET_FLAG(ped, flag))
		PED::SET_PED_RESET_FLAG(ped, flag, false);
	return;
}

bool GetWeightedBool(int chance, int startRange, int endRange)
{
	SET_RANDOM_SEED(GET_GAME_TIMER());
	bool rand = (chance >= GET_RANDOM_INT_IN_RANGE(startRange, endRange));
	return rand;
}

void Print(char* string, int ms = 1)
{
	BEGIN_TEXT_COMMAND_PRINT("STRING");
	ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME(string);
	END_TEXT_COMMAND_PRINT(ms, 1);
	return;
}

void PrintInt(int value, int ms = 1)
{
	BEGIN_TEXT_COMMAND_PRINT("NUMBER");
	ADD_TEXT_COMPONENT_INTEGER(value);
	END_TEXT_COMMAND_PRINT(ms, 1);
	return;
}

void PrintFloat(float value, int ms = 1)
{
	BEGIN_TEXT_COMMAND_PRINT("NUMBER");
	ADD_TEXT_COMPONENT_FLOAT(value, 4);
	END_TEXT_COMMAND_PRINT(ms, 1);
	return;
}