//ScriptHook
#include <natives.h>
// #include <types.h> //Already included in globals.h
//Custom
#include "menuEntries.h"
#include "functions.h"
#include "..\globals.h"
#include "..\script.h"

void SequenceOption(bool should, bool shouldNot, int &status, cSequence &obj)
{
	if (should)
		obj.Start();
	else if (shouldNot && obj.IsActive())
	{
		obj.Stop();
		status = 2;
	}
	else if (!obj.IsActive())
		status = -1;
	else if (obj.ShouldStop())
		status = 2;

	return;
}