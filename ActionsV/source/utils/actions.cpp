//ScriptHook
#include <natives.h>
// #include <types.h> //Already included in globals.h
//Custom
#include "actions.h"
#include "functions.h"
#include "keyboard.h"
#include "..\globals.h"
#include "..\script.h"

cSmokingSequence smokingSequence;
cDrinkingSequence drinkingSequence;

void UpdateSequences()
{
	smokingSequence.Update();
	drinkingSequence.Update();
	return;
}

static bool NoSequenceIsActive()
{
	if (smokingSequence.IsActive() ||
		drinkingSequence.IsActive())
		return false;

	return true;
}

static void StopActiveSequence()
{
	if (smokingSequence.IsActive())
		smokingSequence.Stop();

	if (drinkingSequence.IsActive())
		drinkingSequence.Stop();

	return;
}

void cSequence::Start()
{
	if (!NoSequenceIsActive())
	{
		StopActiveSequence();
		return;
	}

	if (AdditionalChecks(playerPed))
		sequenceState = SEQUENCE_STREAM_ASSETS_IN;

	return;
}

void cSequence::PlayAnim(char *animDict, char *anim, int flag, int duration)
{
	TASK_PLAY_ANIM(playerPed, animDict, anim, 1.5f, -1.5f, duration, flag, 0.0f, false, false, false);
	return;
}

void cSequence::PlayAnimAndWait(char *animDict, char *anim, int flag, int nextState, float startPhase, float blendInSpeed, float blendOutSpeed, bool standStill, int duration)
{
	TASK_PLAY_ANIM(playerPed, animDict, anim, blendInSpeed, blendOutSpeed, duration, flag, startPhase, false, false, false);
	sequenceState = SEQUENCE_WAITING_FOR_ANIMATION_TO_END;
	nextSequenceState = nextState;
	shouldPlayerStandStill = standStill;
	lastAnimDict = animDict;
	lastAnim = anim;
	return;
}

void cSequence::SetPlayerControls()
{
	//Hide Phone and mobile browser
	if (!disabledControlsLastFrame)
	{
		SET_CONTROL_VALUE_NEXT_FRAME(FRONTEND_CONTROL, INPUT_CELLPHONE_CANCEL, 1.0f);
		SET_CONTROL_VALUE_NEXT_FRAME(FRONTEND_CONTROL, INPUT_CURSOR_CANCEL, 1.0f);
		disabledControlsLastFrame = true;
	}
	else
		disabledControlsLastFrame = false;

	if (sequenceState != SEQUENCE_STREAM_ASSETS_IN && sequenceState != SEQUENCE_FLUSH_ASSETS && sequenceState != SEQUENCE_FINISHED)
		DisablePlayerActionsThisFrame();

	if (sequenceState == SEQUENCE_WAITING_FOR_ANIMATION_TO_END)
	{
		if (shouldPlayerStandStill)
			DisablePlayerControlThisFrame();
	}
	else
		shouldPlayerStandStill = false;

	return;
}

void cSequence::SetPedMovementAndReactions() const
{
	if (sequenceState == SEQUENCE_FLUSH_ASSETS || sequenceState == SEQUENCE_FINISHED)
	{
		SET_PED_CAN_PLAY_GESTURE_ANIMS(playerPed, true);
		SET_PED_STEALTH_MOVEMENT(playerPed, true, NULL);
		SET_PED_USING_ACTION_MODE(playerPed, true, 0, NULL);
	}
	else
	{
		SET_PED_CAN_PLAY_GESTURE_ANIMS(playerPed, false);
		SET_PED_STEALTH_MOVEMENT(playerPed, false, NULL);
		SET_PED_USING_ACTION_MODE(playerPed, false, -1, NULL);
	}
	return;
}

//////////////////////////////////SMOKING//////////////////////////////////

void cSmokingSequence::StopAllAnims()
{
	if (IS_ENTITY_PLAYING_ANIM(playerPed, smokeEnterAnimDict, smokeEnterAnim, 3))
		STOP_ANIM_TASK(playerPed, smokeEnterAnimDict, smokeEnterAnim, -2.0f);

	if (IS_ENTITY_PLAYING_ANIM(playerPed, smokeBaseAnimDict, smokeBaseAnim, 3))
		STOP_ANIM_TASK(playerPed, smokeBaseAnimDict, smokeBaseAnim, -2.0f);

	if (IS_ENTITY_PLAYING_ANIM(playerPed, smokeExitAnimDict, smokeExitAnim, 3))
		STOP_ANIM_TASK(playerPed, smokeExitAnimDict, smokeExitAnim, -2.0f);

	REMOVE_ANIM_DICT(smokeEnterAnimDict);
	REMOVE_ANIM_DICT(smokeBaseAnimDict);
	REMOVE_ANIM_DICT(smokeExitAnimDict);
	return;
}

void cSmokingSequence::StopPTFX(int* PTFXHandle)
{
	if (*PTFXHandle != NULL)
	{
		if (DOES_PARTICLE_FX_LOOPED_EXIST(*PTFXHandle))
			STOP_PARTICLE_FX_LOOPED(*PTFXHandle, false);

		*PTFXHandle = NULL;
	}
	return;
}

void cSmokingSequence::StopAllPTFX()
{
	StopPTFX(&cigarettePTFXHandle);
	StopPTFX(&hasExhaledNose);
	REMOVE_NAMED_PTFX_ASSET("core");
	return;
}

void cSmokingSequence::PlayPTFX()
{
	if (sequenceState == STREAM_ASSETS_IN || sequenceState == FLUSH_ASSETS || sequenceState == FINISHED)
	{
		StopAllPTFX();
		return;
	}

	if (!HAS_NAMED_PTFX_ASSET_LOADED("core"))
		REQUEST_NAMED_PTFX_ASSET("core");

	// offsets can be found inside scenarios
	const float enterAnimTime = GET_ENTITY_ANIM_CURRENT_TIME(playerPed, smokeEnterAnimDict, smokeEnterAnim);
	if (enterAnimTime >= 0.4826087f && cigarettePTFXHandle == NULL)
	{
		USE_PARTICLE_FX_ASSET("core");
		cigarettePTFXHandle = START_PARTICLE_FX_LOOPED_ON_PED_BONE(ANM_CIG_SMOKE, playerPed, -0.08f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, BONETAG_PH_R_HAND, 1.0f, false, false, false);
	}

	const float baseAnimTime = GET_ENTITY_ANIM_CURRENT_TIME(playerPed, smokeBaseAnimDict, smokeBaseAnim);
	if (baseAnimTime < 0.7965517f)
		hasExhaledNose = NULL;

	if (baseAnimTime >= 0.7965517f && hasExhaledNose == NULL)
	{
		USE_PARTICLE_FX_ASSET("core");
		hasExhaledNose = START_PARTICLE_FX_NON_LOOPED_ON_PED_BONE(ANM_CIG_EXHALE_NSE, playerPed, 0.02f, 0.16f, 0.0f, 0.0f, 0.0f, 0.0f, BONETAG_HEAD, 1.0f, false, false, false);
	}
	return;
}

void cSmokingSequence::PlaySequence()
{
	const int rightHandID = GET_PED_BONE_INDEX(playerPed, BONETAG_PH_R_HAND);

	SetPlayerControls(); //Player control should be disabled here and not during the sequence

	switch (sequenceState)
	{
	case INITIALIZED:
		item = CreateObject(cigaretteHash);
		SET_ENTITY_AS_MISSION_ENTITY(item, true, true);

		if (!IS_ENTITY_ATTACHED(item)) //&& GET_ENTITY_ANIM_CURRENT_TIME(playerPed, smokeBaseAnimDict, smokeBaseAnim) > 0.225f)
			ATTACH_ENTITY_TO_ENTITY(item, playerPed, rightHandID, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, true, true, false, false, 2, true);

		PlayAnimAndWait(smokeEnterAnimDict, smokeEnterAnim, upperSecondaryAF | AF_HOLD_LAST_FRAME, LOOP);
		break;
	case WAITING_FOR_ANIMATION_TO_END:
		if (!IS_ENTITY_PLAYING_ANIM(playerPed, lastAnimDict, lastAnim, 3))
			sequenceState = nextSequenceState;
		break;
	case LOOP:
		if (!IS_ENTITY_PLAYING_ANIM(playerPed, smokeEnterAnimDict, smokeEnterAnim, 3))
			PlayAnimAndWait(smokeBaseAnimDict, smokeBaseAnim, upperSecondaryAF | AF_HOLD_LAST_FRAME, LOOP, 0.388f);
		break;
	case EXITING:
		PlayAnimAndWait(smokeExitAnimDict, smokeExitAnim, upperSecondaryAF, FLUSH_ASSETS);
		break;
	case FLUSH_ASSETS:
		DeleteObject(&item);
		REMOVE_ANIM_DICT(smokeEnterAnimDict);
		REMOVE_ANIM_DICT(smokeBaseAnimDict);
		REMOVE_ANIM_DICT(smokeExitAnimDict);
		SET_MODEL_AS_NO_LONGER_NEEDED(cigaretteHash);
		sequenceState = FINISHED;
		break;
	case STREAM_ASSETS_IN:
		if (RequestModel(cigaretteHash) && RequestAnimDict(smokeEnterAnimDict) &&
			RequestAnimDict(smokeBaseAnimDict) && RequestAnimDict(smokeExitAnimDict))
		{
			sequenceState = INITIALIZED;
			nextSequenceState = NULL;
			shouldPlayerStandStill = false;
			lastAnimDict = NULL;
			lastAnim = NULL;
			item = NULL;
		}
		break;
	}

	//Play PTFXs
	PlayPTFX();

	//Disable ped gestures and block non-player peds from reacting to temporary events
	SetPedMovementAndReactions();
	return;
}

bool cSmokingSequence::GetAnimHold(char** animDict, char** anim)
{
	if (IS_ENTITY_PLAYING_ANIM(playerPed, smokeEnterAnimDict, smokeEnterAnim, 3) &&
		GET_ENTITY_ANIM_CURRENT_TIME(playerPed, smokeEnterAnimDict, smokeEnterAnim) == 1.0f)
	{
		*animDict = smokeEnterAnimDict;
		*anim = smokeEnterAnim;
	}
	else if (IS_ENTITY_PLAYING_ANIM(playerPed, smokeBaseAnimDict, smokeBaseAnim, 3) &&
		GET_ENTITY_ANIM_CURRENT_TIME(playerPed, smokeBaseAnimDict, smokeBaseAnim) == 1.0f)
	{
		*animDict = smokeBaseAnimDict;
		*anim = smokeBaseAnim;
	}
	else
		return false;

	return true;
}

void cSmokingSequence::SetState(int state)
{
	char* animDict = ""; char* anim = "";
	if (!GetAnimHold(&animDict, &anim))
		return;

	STOP_ANIM_TASK(playerPed, animDict, anim, -2.0f);

	if (state == EXITING && sequenceState != EXITING && sequenceState != FLUSH_ASSETS)
		sequenceState = EXITING;
	else
		sequenceState = state;

	return;
}

void cSmokingSequence::UpdateControls() 
{
	char* animDict = ""; char* anim = "";
	if (!GetAnimHold(&animDict, &anim))
		return;

	AddScaleformInstructionalButton(control, input, "Smoke (hold to stop)", true);
	RunScaleformInstructionalButtons();

	if (IS_DISABLED_CONTROL_JUST_PRESSED(control, input))
	{
		controlTimer.Set(0);
		return;
	}

	if (controlTimer.Get() < holdTime)
	{
		if (IS_DISABLED_CONTROL_JUST_RELEASED(control, input))
			SetState(LOOP);
	}
	else if (IS_DISABLED_CONTROL_PRESSED(control, input))
		SetState(EXITING);

	return;
}

void cSmokingSequence::ForceStop()
{
	if (sequenceState == FLUSH_ASSETS || sequenceState == FINISHED)
		return;

	StopAllAnims();
	StopAllPTFX();
	DeleteObject(&item);
	shouldStopSequence = false;
	sequenceState = FLUSH_ASSETS;
	PlaySequence();
	return;
}

void cSmokingSequence::Update()
{
	if (sequenceState != FINISHED)
	{
		if (!AdditionalChecks(playerPed))
		{
			ForceStop();
			return;
		}

		PlaySequence();
		UpdateControls();
		if (shouldStopSequence)
		{
			if (stopTimer.Get() > maxStopTimer)
				ForceStop();

			SetState(EXITING);
		}
		else
			stopTimer.Set(0);

		return;
	}

	shouldStopSequence = false; //Reset var
	DeleteObject(&item); //Force delete old item
	return;
}

//////////////////////////////////DRINKING//////////////////////////////////

void cDrinkingSequence::StopAllAnims()
{
	if (IS_ENTITY_PLAYING_ANIM(playerPed, drinkingAnimDict, drinkingEnterAnim, 3))
		STOP_ANIM_TASK(playerPed, drinkingAnimDict, drinkingEnterAnim, -2.0f);

	if (IS_ENTITY_PLAYING_ANIM(playerPed, drinkingAnimDict, drinkingBaseAnim, 3))
		STOP_ANIM_TASK(playerPed, drinkingAnimDict, drinkingBaseAnim, -2.0f);

	if (IS_ENTITY_PLAYING_ANIM(playerPed, drinkingAnimDict, drinkingExitAnim, 3))
		STOP_ANIM_TASK(playerPed, drinkingAnimDict, drinkingExitAnim, -2.0f);

	REMOVE_ANIM_DICT(drinkingAnimDict);
	return;
}

void cDrinkingSequence::PlaySequence()
{
	const int leftHandID = GET_PED_BONE_INDEX(playerPed, BONETAG_PH_L_HAND);

	SetPlayerControls(); //Player control should be disabled here and not during the sequence

	switch (sequenceState)
	{
	case INITIALIZED:
		item = CreateObject(beerHash);
		SET_ENTITY_AS_MISSION_ENTITY(item, true, true);
		ATTACH_ENTITY_TO_ENTITY(item, playerPed, leftHandID, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, true, true, false, false, 2, true);
		PlayAnimAndWait(drinkingAnimDict, drinkingEnterAnim, upperSecondaryAF, DRINK, 0.35f, 1.5f, -0.1f);
		break;
	case WAITING_FOR_ANIMATION_TO_END:
		if (!IS_ENTITY_PLAYING_ANIM(playerPed, lastAnimDict, lastAnim, 3))
			sequenceState = nextSequenceState;

		if (IS_ENTITY_PLAYING_ANIM(playerPed, drinkingAnimDict, drinkingEnterAnim, 3))
			SET_ENTITY_ANIM_SPEED(playerPed, drinkingAnimDict, drinkingEnterAnim, 0.7f);
		break;
	case ENTER_DRINK:
		if (!IS_ENTITY_PLAYING_ANIM(playerPed, drinkingAnimDict, drinkingExitAnim, 3))
			PlayAnimAndWait(drinkingAnimDict, drinkingEnterAnim, upperSecondaryAF, DRINK, 0.35f, 1.5f, -0.1f);
		break;
	case DRINK:
		if (!IS_ENTITY_PLAYING_ANIM(playerPed, drinkingAnimDict, drinkingEnterAnim, 3))
			PlayAnimAndWait(drinkingAnimDict, drinkingBaseAnim, upperSecondaryAF, HOLD, 0.0f, 0.5f, -0.5f);
		break;
	case HOLD:
		if (!IS_ENTITY_PLAYING_ANIM(playerPed, drinkingAnimDict, drinkingBaseAnim, 3))
			PlayAnimAndWait(drinkingAnimDict, drinkingExitAnim, upperSecondaryAF | AF_HOLD_LAST_FRAME, ENTER_DRINK, 0.0f, 0.5f);
		break;
	case EXITING:
		StopAllAnims();
		sequenceState = FLUSH_ASSETS;
		break;
	case FLUSH_ASSETS:
		REMOVE_ANIM_DICT(drinkingAnimDict);
		DETACH_ENTITY(item, false, false);
		SET_ENTITY_AS_NO_LONGER_NEEDED(&item);
		SET_MODEL_AS_NO_LONGER_NEEDED(beerHash);
		sequenceState = FINISHED;
		break;
	case STREAM_ASSETS_IN:
		if (RequestModel(beerHash) && RequestAnimDict(drinkingAnimDict))
		{
			sequenceState = INITIALIZED;
			nextSequenceState = NULL;
			shouldPlayerStandStill = false;
			lastAnimDict = NULL;
			lastAnim = NULL;
			item = NULL;
		}
		break;
	}

	//Disable ped gestures and block non-player peds from reacting to temporary events
	SetPedMovementAndReactions();
	return;
}

bool cDrinkingSequence::GetAnimHold(char** animDict, char** anim)
{
	if (IS_ENTITY_PLAYING_ANIM(playerPed, drinkingAnimDict, drinkingExitAnim, 3) &&
		GET_ENTITY_ANIM_CURRENT_TIME(playerPed, drinkingAnimDict, drinkingExitAnim) == 1.0f)
	{
		*animDict = drinkingAnimDict;
		*anim = drinkingExitAnim;
	}
	else
		return false;

	return true;
}

void cDrinkingSequence::SetState(int state)
{
	char* animDict = ""; char* anim = "";
	if (!GetAnimHold(&animDict, &anim))
		return;

	STOP_ANIM_TASK(playerPed, animDict, anim, -2.0f);

	if (state == EXITING && sequenceState != EXITING && sequenceState != FLUSH_ASSETS)
		sequenceState = EXITING;
	else
		sequenceState = state;

	return;
}

void cDrinkingSequence::UpdateControls()
{
	char* animDict = ""; char* anim = "";
	if (!GetAnimHold(&animDict, &anim))
		return;

	AddScaleformInstructionalButton(control, input, "Drink (hold to stop)", true);
	RunScaleformInstructionalButtons();

	if (IS_DISABLED_CONTROL_JUST_PRESSED(control, input))
	{
		controlTimer.Set(0);
		return;
	}

	if (controlTimer.Get() < holdTime)
	{
		if (IS_DISABLED_CONTROL_JUST_RELEASED(control, input))
			SetState(ENTER_DRINK);
	}
	else if (IS_DISABLED_CONTROL_PRESSED(control, input))
		SetState(EXITING);

	return;
}

void cDrinkingSequence::ForceStop()
{
	if (sequenceState == FLUSH_ASSETS || sequenceState == FINISHED)
		return;

	StopAllAnims();
	DeleteObject(&item);
	shouldStopSequence = false;
	sequenceState = FLUSH_ASSETS;
	PlaySequence();
	return;
}

void cDrinkingSequence::Update()
{
	if (sequenceState != FINISHED)
	{
		if (!AdditionalChecks(playerPed))
		{
			ForceStop();
			return;
		}

		PlaySequence();
		UpdateControls();
		if (shouldStopSequence)
		{
			if (stopTimer.Get() > maxStopTimer)
				ForceStop();

			SetState(EXITING);
		}
		else
			stopTimer.Set(0);

		return;
	}

	shouldStopSequence = false; //Reset var
	return;
}