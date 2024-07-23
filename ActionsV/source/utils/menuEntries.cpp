//ScriptHook
#include <natives.h>
// #include <types.h> //Already included in globals.h
//Custom
#include "actions.h"
#include "functions.h"
#include "..\globals.h"
#include "..\script.h"

//Smoke
void SmokeOption(bool shouldSmoke, bool shouldNotSmoke, int &smokingStatus)
{
	if (shouldSmoke)
		smokingSequence.Start();
	else if (shouldNotSmoke && smokingSequence.IsActive())
	{
		smokingSequence.Stop();
		smokingStatus = 2;
	}
	else if (!smokingSequence.IsActive())
		smokingStatus = -1;
	else if (smokingSequence.ShouldStop())
		smokingStatus = 2;

	return;
}

//Drink
void DrinkOption(bool shouldDrink, bool shouldNotDrink, int &drinkingStatus)
{
	if (shouldDrink)
		drinkingSequence.Start();
	else if (shouldNotDrink && drinkingSequence.IsActive())
	{
		drinkingSequence.Stop();
		drinkingStatus = 2;
	}
	else if (!drinkingSequence.IsActive())
		drinkingStatus = -1;
	else if (drinkingSequence.ShouldStop())
		drinkingStatus = 2;

	return;
}