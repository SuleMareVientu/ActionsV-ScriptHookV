//ScriptHook
#include <natives.h>
// #include <types.h> //Already included in globals.h
//Custom
#include "actions.h"
#include "functions.h"
#include "keyboard.h"
#include "..\globals.h"
#include "..\script.h"

static bool NoSequenceActive();

namespace SmokingSequence {
enum eSequenceState
{
	STREAM_ASSETS_IN,
	INITIALIZED,
	WAITING_FOR_ANIMATION_TO_END,
	LOOP,
	EXITING,
	FLUSH_ASSETS,
	FINISHED
};

constexpr Hash cigaretteHash = 2017086435;		//Prop_AMB_Ciggy_01
constexpr char* smokeBaseAnimDict = "amb@world_human_smoking@male@male_a@base";
constexpr char* smokeBaseAnim = "base";

constexpr char* smokeEnterAnimDict = "amb@world_human_smoking@male@male_a@enter";
constexpr char* smokeEnterAnim = "enter";

constexpr char* smokeExitAnimDict = "amb@world_human_smoking@male@male_a@exit";
constexpr char* smokeExitAnim = "exit";

constexpr char* smokeIdleAnimDict = "amb@world_human_smoking@male@male_a@idle_a";
constexpr char* smokeIdleAAnim = "idle_a";
constexpr char* smokeIdleBAnim = "idle_b";
constexpr char* smokeIdleCAnim = "idle_c";

constexpr char* ANM_CIG_SMOKE = "ent_anim_cig_smoke";
constexpr char* ANM_CIG_EXHALE_MTH = "ent_anim_cig_exhale_mth";
constexpr char* ANM_CIG_EXHALE_NSE = "ent_anim_cig_exhale_nse";
constexpr char* ANM_CIG_SMOKE_CAR = "ent_anim_cig_smoke_car";
constexpr char* ANM_CIG_EXHALE_MTH_CAR = "ent_anim_cig_exhale_mth_car";
constexpr char* ANM_CIG_EXHALE_NSE_CAR = "ent_anim_cig_exhale_nse_car";

// Variables
bool disabledControlsLastFrame = false;
bool shouldPlayerStandStill = false;
int sequenceState = FINISHED;
int nextSequenceState = NULL;
int forcedSequenceState = -2;
char* lastAnimDict = NULL;
char* lastAnim = NULL;
Object item = NULL;

bool IsSequenceActive()
{
	if (sequenceState == FINISHED)
		return false;

	return true;
}

void PlayAnim(char* animDict, char* anim, int flag, int duration = -1)
{
	TASK_PLAY_ANIM(playerPed, animDict, anim, 1.5f, -1.5f, duration, flag, 0.0f, false, false, false);
	return;
}

void PlayAnimAndWait(char* animDict, char* anim, int flag, int nextState, float startPhase = 0.0f, bool standStill = false, int duration = -1)
{
	TASK_PLAY_ANIM(playerPed, animDict, anim, 1.5f, -1.5f, duration, flag, startPhase, false, false, false);
	sequenceState = WAITING_FOR_ANIMATION_TO_END;
	nextSequenceState = nextState;
	shouldPlayerStandStill = standStill;
	lastAnimDict = animDict;
	lastAnim = anim;
	return;
}

void SetPlayerControls()
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

	if (sequenceState != STREAM_ASSETS_IN && sequenceState != FLUSH_ASSETS && sequenceState != FINISHED)
		DisablePlayerActionsThisFrame();

	if (sequenceState == WAITING_FOR_ANIMATION_TO_END)
	{
		if (shouldPlayerStandStill)
			DisablePlayerControlThisFrame();
	}
	else
		shouldPlayerStandStill = false;

	return;
}

void SetPedMovementAndReactions()
{
	if (sequenceState == FLUSH_ASSETS || sequenceState == FINISHED)
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

void StopAllAnims()
{
	if (IS_ENTITY_PLAYING_ANIM(playerPed, smokeEnterAnimDict, smokeEnterAnim, 3))
		STOP_ANIM_TASK(playerPed, smokeEnterAnimDict, smokeEnterAnim, -2.0f);

	if (IS_ENTITY_PLAYING_ANIM(playerPed, smokeBaseAnimDict, smokeBaseAnim, 3))
		STOP_ANIM_TASK(playerPed, smokeBaseAnimDict, smokeBaseAnim, -2.0f);

	if (IS_ENTITY_PLAYING_ANIM(playerPed, smokeExitAnimDict, smokeExitAnim, 3))
		STOP_ANIM_TASK(playerPed, smokeExitAnimDict, smokeExitAnim, -2.0f);

	return;
}

void StopPTFX(int* PTFXHandle)
{
	if (*PTFXHandle != NULL)
	{
		if (DOES_PARTICLE_FX_LOOPED_EXIST(*PTFXHandle))
			STOP_PARTICLE_FX_LOOPED(*PTFXHandle, false);

		*PTFXHandle = NULL;
	}
	return;
}

int cigarettePTFXHandle = NULL;
int hasExhaledNose = NULL;
void PlaySmokingPTFX()
{
	if (sequenceState == STREAM_ASSETS_IN || sequenceState == FLUSH_ASSETS || sequenceState == FINISHED)
	{
		StopPTFX(&cigarettePTFXHandle);
		StopPTFX(&hasExhaledNose);
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

void PlaySmokingSequence()
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
	PlaySmokingPTFX();

	//Disable ped gestures and block non-player peds from reacting to temporary events
	SetPedMovementAndReactions();
	return;
}

void StartSequence()
{
	if (NoSequenceActive() && AdditionalChecks(playerPed))
	{
		sequenceState = STREAM_ASSETS_IN;
		PlaySmokingSequence();
	}
	return;
}

bool GetAnimHold(char** animDict, char** anim)
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

void SetSequenceState(int state, bool shouldForceState)
{
	if (shouldForceState)
		forcedSequenceState = state;

	char* animDict = ""; char* anim = "";
	if (!GetAnimHold(&animDict, &anim))
		return;

	STOP_ANIM_TASK(playerPed, animDict, anim, -2.0f);

	if (state <= -1 && sequenceState != EXITING && sequenceState != FLUSH_ASSETS)
		sequenceState = EXITING;
	else
		sequenceState = state;

	return;
}

Timer controlTimer;
constexpr int holdTime = 250;
constexpr int smokingControl = PLAYER_CONTROL;
constexpr int smokingInput = INPUT_JUMP;
void UpdateControls()
{
	char* animDict = ""; char* anim = "";
	if (!GetAnimHold(&animDict, &anim))
		return;

	AddScaleformInstructionalButton(smokingControl, smokingInput, "Smoke (hold to stop)", true);
	RunScaleformInstructionalButtons();

	if (IS_DISABLED_CONTROL_JUST_PRESSED(smokingControl, smokingInput))
	{
		controlTimer.Set(0);
		return;
	}

	if (controlTimer.Get() < holdTime)
	{
		if (IS_DISABLED_CONTROL_JUST_RELEASED(smokingControl, smokingInput))
			SetSequenceState(LOOP);
	}
	else if (IS_DISABLED_CONTROL_PRESSED(smokingControl, smokingInput))
		SetSequenceState(EXITING);

	return;
}

void UpdateSequence()
{
	if (sequenceState != FINISHED && !AdditionalChecks(playerPed))
	{
		StopAllAnims();
		sequenceState = FLUSH_ASSETS;
		forcedSequenceState = -2;
		return;
	}

	if (forcedSequenceState == sequenceState || sequenceState == FINISHED)
		forcedSequenceState = -2;

	if (sequenceState != FINISHED)
	{
		PlaySmokingSequence();
		UpdateControls();
		if (forcedSequenceState > -2)
			SetSequenceState(forcedSequenceState);

		return;
	}

	DeleteObject(&item); //Force delete old item
	return;
}
}	// END namespace SmokingSequence

namespace DrinkingSequence {
enum eSequenceState
{
	STREAM_ASSETS_IN,
	INITIALIZED,
	WAITING_FOR_ANIMATION_TO_END,
	ENTER_DRINK,
	DRINK,
	HOLD,
	EXITING,
	FLUSH_ASSETS,
	FINISHED
};

constexpr Hash beerHash = 683570518;		//Prop_AMB_Beer_Bottle
constexpr char* drinkingAnimDict = "mp_player_intdrink";
constexpr char* drinkingEnterAnim = "intro_bottle";
constexpr char* drinkingBaseAnim = "loop_bottle";
constexpr char* drinkingExitAnim = "outro_bottle";

// Variables
bool disabledControlsLastFrame = false;
bool shouldPlayerStandStill = false;
int sequenceState = FINISHED;
int nextSequenceState = NULL;
int forcedSequenceState = -2;
char* lastAnimDict = NULL;
char* lastAnim = NULL;
Object item = NULL;

bool IsSequenceActive()
{
	if (sequenceState == FINISHED)
		return false;

	return true;
}

void PlayAnim(char* animDict, char* anim, int flag, int duration = -1)
{
	TASK_PLAY_ANIM(playerPed, animDict, anim, 1.5f, -1.5f, duration, flag, 0.0f, false, false, false);
	return;
}

void PlayAnimAndWait(char* animDict, char* anim, int flag, int nextState, float startPhase = 0.0f, float blendInSpeed = 1.5f, float blendOutSpeed = -1.5f, bool standStill = false, int duration = -1)
{
	TASK_PLAY_ANIM(playerPed, animDict, anim, blendInSpeed, blendOutSpeed, duration, flag, startPhase, false, false, false);
	sequenceState = WAITING_FOR_ANIMATION_TO_END;
	nextSequenceState = nextState;
	shouldPlayerStandStill = standStill;
	lastAnimDict = animDict;
	lastAnim = anim;
	return;
}

void SetPlayerControls()
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

	if (sequenceState != STREAM_ASSETS_IN && sequenceState != FLUSH_ASSETS && sequenceState != FINISHED)
		DisablePlayerActionsThisFrame();

	if (sequenceState == WAITING_FOR_ANIMATION_TO_END)
	{
		if (shouldPlayerStandStill)
			DisablePlayerControlThisFrame();
	}
	else
		shouldPlayerStandStill = false;

	return;
}

void SetPedMovementAndReactions()
{
	if (sequenceState == FLUSH_ASSETS || sequenceState == FINISHED)
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

void StopAllAnims()
{
	if (IS_ENTITY_PLAYING_ANIM(playerPed, drinkingAnimDict, drinkingEnterAnim, 3))
		STOP_ANIM_TASK(playerPed, drinkingAnimDict, drinkingEnterAnim, -2.0f);

	if (IS_ENTITY_PLAYING_ANIM(playerPed, drinkingAnimDict, drinkingBaseAnim, 3))
		STOP_ANIM_TASK(playerPed, drinkingAnimDict, drinkingBaseAnim, -2.0f);

	if (IS_ENTITY_PLAYING_ANIM(playerPed, drinkingAnimDict, drinkingExitAnim, 3))
		STOP_ANIM_TASK(playerPed, drinkingAnimDict, drinkingExitAnim, -2.0f);

	return;
}

void PlayDrinkingSequence()
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

void StartSequence()
{
	if (NoSequenceActive() && AdditionalChecks(playerPed))
	{
		sequenceState = STREAM_ASSETS_IN;
		PlayDrinkingSequence();
	}
	return;
}

bool GetAnimHold(char** animDict, char** anim)
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

void SetSequenceState(int state, bool shouldForceState)
{
	if (shouldForceState)
		forcedSequenceState = state;

	char* animDict = ""; char* anim = "";
	if (!GetAnimHold(&animDict, &anim))
		return;

	STOP_ANIM_TASK(playerPed, animDict, anim, -2.0f);

	if (state <= -1 && sequenceState != EXITING && sequenceState != FLUSH_ASSETS)
		sequenceState = EXITING;
	else
		sequenceState = state;
	return;
}

Timer controlTimer;
constexpr int holdTime = 250;
constexpr int drinkingControl = PLAYER_CONTROL;
constexpr int drinkingInput = INPUT_JUMP;
void UpdateControls()
{
	char* animDict = ""; char* anim = "";
	if (!GetAnimHold(&animDict, &anim))
		return;

	AddScaleformInstructionalButton(drinkingControl, drinkingInput, "Drink (hold to stop)", true);
	RunScaleformInstructionalButtons();

	if (IS_DISABLED_CONTROL_JUST_PRESSED(drinkingControl, drinkingInput))
	{
		controlTimer.Set(0);
		return;
	}

	if (controlTimer.Get() < holdTime)
	{
		if (IS_DISABLED_CONTROL_JUST_RELEASED(drinkingControl, drinkingInput))
			SetSequenceState(ENTER_DRINK);
	}
	else if (IS_DISABLED_CONTROL_PRESSED(drinkingControl, drinkingInput))
		SetSequenceState(EXITING);

	return;
}

void UpdateSequence()
{
	if (sequenceState != FINISHED && !AdditionalChecks(playerPed))
	{
		StopAllAnims();
		sequenceState = FLUSH_ASSETS;
		forcedSequenceState = -2;
		return;
	}

	if (forcedSequenceState == sequenceState || sequenceState == FINISHED)
		forcedSequenceState = -2;

	if (sequenceState != FINISHED)
	{
		PlayDrinkingSequence();
		UpdateControls();
		if (forcedSequenceState > -2)
			SetSequenceState(forcedSequenceState);

		return;
	}

	DeleteObject(&item); //Force delete old item
	return;
}
}	// END namespace DrinkingSequence

static bool NoSequenceActive()
{
	if (SmokingSequence::IsSequenceActive() ||
		DrinkingSequence::IsSequenceActive())
		return false;

	return true;
}

void UpdateSequences()
{
	SmokingSequence::UpdateSequence();
	DrinkingSequence::UpdateSequence();
	return;
}