//ScriptHook
#include <natives.h>
// #include <types.h> //Already included in globals.h
//Custom
#include "actions.h"
#include "functions.h"
#include "..\globals.h"
#include "..\script.h"

//Smoke
void SmokeOption(bool should, bool shouldNot, int &status)
{
	if (should)
		smokingSequence.Start();
	else if (shouldNot && smokingSequence.IsActive())
	{
		smokingSequence.Stop();
		status = 2;
	}
	else if (!smokingSequence.IsActive())
		status = -1;
	else if (smokingSequence.ShouldStop())
		status = 2;

	return;
}

//Drink
void DrinkOption(bool should, bool shouldNot, int &status)
{
	if (should)
		drinkingSequence.Start();
	else if (shouldNot && drinkingSequence.IsActive())
	{
		drinkingSequence.Stop();
		status = 2;
	}
	else if (!drinkingSequence.IsActive())
		status = -1;
	else if (drinkingSequence.ShouldStop())
		status = 2;

	return;
}

//Leaf Blower
void LeafBlowerOption(bool should, bool shouldNot, int &status)
{
	if (should)
		leafBlowerSequence.Start();
	else if (shouldNot && leafBlowerSequence.IsActive())
	{
		leafBlowerSequence.Stop();
		status = 2;
	}
	else if (!leafBlowerSequence.IsActive())
		status = -1;
	else if (leafBlowerSequence.ShouldStop())
		status = 2;

	return;
}

//Jog
void JogOption(bool should, bool shouldNot, int &status)
{
	if (should)
		jogSequence.Start();
	else if (shouldNot && jogSequence.IsActive())
	{
		jogSequence.Stop();
		status = 2;
	}
	else if (!jogSequence.IsActive())
		status = -1;
	else if (jogSequence.ShouldStop())
		status = 2;

	return;
}