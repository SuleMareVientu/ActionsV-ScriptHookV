//ScriptHook
#include <natives.h>
// #include <types.h> //Already included in globals.h
//Custom
#include "actions.h"
#include "functions.h"
#include "..\globals.h"
#include "..\script.h"

enum eActiveSequence
{
	SMOKING,
	DRINKING
};

//Audio
static constexpr char* AmbDialogueHi = "GENERIC_HI";
static constexpr char* AmbDialogueBye = "GENERIC_BYE";
static constexpr char* AmbDialogueThanks = "GENERIC_THANKS";
static constexpr char* AmbDialogueBuy = "GENERIC_BUY";
static constexpr char* AmbDialogueEat = "GENERIC_EAT";

static constexpr int smokingControl = INPUT_CONTEXT;
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
char* lastAnimDict = NULL;
char* lastAnim = NULL;
Object item = NULL;

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

void SetPedReactions()
{
	if (sequenceState == FINISHED)
		SET_PED_CAN_PLAY_GESTURE_ANIMS(playerPed, true);
	else
		SET_PED_CAN_PLAY_GESTURE_ANIMS(playerPed, false);
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
		DeleteObject(item);
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
	SetPedReactions();
	return;
}

void StartSequence()
{
	sequenceState = STREAM_ASSETS_IN;
	PlaySmokingSequence();
	return;
}

Timer controlTimer;
constexpr int holdTime = 250;
void UpdateSequence()
{
	if (!DOES_ENTITY_EXIST(playerPed) || IS_ENTITY_DEAD(playerPed, false) || IS_PED_DEAD_OR_DYING(playerPed, true) || IS_PED_INJURED(playerPed)
		|| IS_PED_IN_MELEE_COMBAT(playerPed) || COUNT_PEDS_IN_COMBAT_WITH_TARGET(playerPed) > 0 || IS_PED_USING_ANY_SCENARIO(playerPed))
	{
		sequenceState = FINISHED;
		return;
	}

	if (sequenceState != FINISHED)
	{
		PlaySmokingSequence();

		GET_ENTITY_ANIM_CURRENT_TIME(playerPed, smokeBaseAnimDict, smokeBaseAnim);

		if (IS_DISABLED_CONTROL_JUST_PRESSED(PLAYER_CONTROL, smokingControl))
			controlTimer.Set(0);
		else if (IS_ENTITY_PLAYING_ANIM(playerPed, smokeBaseAnimDict, smokeBaseAnim, 3) &&
				GET_ENTITY_ANIM_CURRENT_TIME(playerPed, smokeBaseAnimDict, smokeBaseAnim) == 1.0f)
		{
			if (controlTimer.Get() < holdTime)
			{
				if (IS_DISABLED_CONTROL_JUST_RELEASED(PLAYER_CONTROL, smokingControl))
				{
					STOP_ANIM_TASK(playerPed, smokeBaseAnimDict, smokeBaseAnim, -2.0f);
					sequenceState = LOOP;
				}
			}
			else if (IS_DISABLED_CONTROL_PRESSED(PLAYER_CONTROL, smokingControl))
			{
				STOP_ANIM_TASK(playerPed, smokeBaseAnimDict, smokeBaseAnim, -2.0f);
				sequenceState = EXITING;
			}
		}
		else if (IS_ENTITY_PLAYING_ANIM(playerPed, smokeEnterAnimDict, smokeEnterAnim, 3) &&
				GET_ENTITY_ANIM_CURRENT_TIME(playerPed, smokeEnterAnimDict, smokeEnterAnim) == 1.0f)
		{
			if (controlTimer.Get() < holdTime)
			{
				if (IS_DISABLED_CONTROL_JUST_RELEASED(PLAYER_CONTROL, smokingControl))
				{
					STOP_ANIM_TASK(playerPed, smokeEnterAnimDict, smokeEnterAnim, -2.0f);
					sequenceState = LOOP;
				}
			}
			else if (IS_DISABLED_CONTROL_PRESSED(PLAYER_CONTROL, smokingControl))
			{
				STOP_ANIM_TASK(playerPed, smokeEnterAnimDict, smokeEnterAnim, -2.0f);
				sequenceState = EXITING;
			}
		}
		return;
	}

	DeleteObject(item); //Force delete old item

	if (IS_DISABLED_CONTROL_JUST_PRESSED(FRONTEND_CONTROL, smokingControl) && AdditionalChecks(playerPed))
		StartSequence();
	return;
}
}	// END namespace SmokingSequence

//namespace DrinkingSequence{}