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
	{
		if (!NoSequenceIsActive())
			StopActiveSequence();

		SmokingSequence::StartSequence();
	}
	else if (shouldNotSmoke)
		SmokingSequence::SetSequenceState(-1, true);
	else if (!SmokingSequence::IsSequenceActive())
		smokingStatus = -1;

	if (SmokingSequence::GetForcedSequenceState() == -1)
		smokingStatus = 2;

	return;
}

//Drink
void DrinkOption(bool shouldDrink, bool shouldNotDrink, int &drinkingStatus)
{
	if (shouldDrink)
	{
		if (!NoSequenceIsActive())
			StopActiveSequence();

		DrinkingSequence::StartSequence();
	}
	else if (shouldNotDrink)
		DrinkingSequence::SetSequenceState(-1, true);
	else if (!DrinkingSequence::IsSequenceActive())
		drinkingStatus = -1;

	if (DrinkingSequence::GetForcedSequenceState() == -1)
		drinkingStatus = 2;

	return;
}