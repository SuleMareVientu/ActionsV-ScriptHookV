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
cLeafBlowerSequence leafBlowerSequence;
cJogSequence jogSequence;

static constexpr char* CORE_PTFX_ASSET = "core";

void UpdateSequences()
{
	smokingSequence.Update();
	drinkingSequence.Update();
	leafBlowerSequence.Update();
	jogSequence.Update();
	return;
}

static bool NoSequenceIsActive()
{
	if (smokingSequence.IsActive() ||
		drinkingSequence.IsActive() ||
		leafBlowerSequence.IsActive() ||
		jogSequence.IsActive())
		return false;

	return true;
}

static void StopActiveSequence()
{
	if (smokingSequence.IsActive())
		smokingSequence.Stop();

	if (drinkingSequence.IsActive())
		drinkingSequence.Stop();

	if (leafBlowerSequence.IsActive())
		leafBlowerSequence.Stop();

	if (jogSequence.IsActive())
		jogSequence.Stop();

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
		SET_PED_CAN_PLAY_AMBIENT_ANIMS(playerPed, true);
		SET_PED_CAN_PLAY_AMBIENT_BASE_ANIMS(playerPed, true);
		SET_PED_CAN_PLAY_AMBIENT_IDLES(playerPed, false, false);
		SET_PED_STEALTH_MOVEMENT(playerPed, true, NULL);
		SET_PED_USING_ACTION_MODE(playerPed, true, 0, NULL);
	}
	else
	{
		SET_PED_CAN_PLAY_GESTURE_ANIMS(playerPed, false);
		SET_PED_CAN_PLAY_AMBIENT_ANIMS(playerPed, false);
		SET_PED_CAN_PLAY_AMBIENT_BASE_ANIMS(playerPed, false);
		SET_PED_CAN_PLAY_AMBIENT_IDLES(playerPed, true, true);
		SET_PED_STEALTH_MOVEMENT(playerPed, false, NULL);
		SET_PED_USING_ACTION_MODE(playerPed, false, -1, NULL);
	}
	return;
}

//////////////////////////////////SMOKING//////////////////////////////////
static constexpr int cigaretteHash = 0x783A4BE3;		//Prop_AMB_Ciggy_01
static constexpr char* smokeBaseAnimDict = "amb@world_human_smoking@male@male_a@base";
static constexpr char* smokeBaseAnim = "base";

static constexpr char* smokeEnterAnimDict = "amb@world_human_smoking@male@male_a@enter";
static constexpr char* smokeEnterAnim = "enter";

static constexpr char* smokeExitAnimDict = "amb@world_human_smoking@male@male_a@exit";
static constexpr char* smokeExitAnim = "exit";

static constexpr char* smokeIdleAnimDict = "amb@world_human_smoking@male@male_a@idle_a";
static constexpr char* smokeIdleAAnim = "idle_a";
static constexpr char* smokeIdleBAnim = "idle_b";
static constexpr char* smokeIdleCAnim = "idle_c";

static constexpr char* ANM_CIG_SMOKE = "ent_anim_cig_smoke";
static constexpr char* ANM_CIG_EXHALE_MTH = "ent_anim_cig_exhale_mth";
static constexpr char* ANM_CIG_EXHALE_NSE = "ent_anim_cig_exhale_nse";
static constexpr char* ANM_CIG_SMOKE_CAR = "ent_anim_cig_smoke_car";
static constexpr char* ANM_CIG_EXHALE_MTH_CAR = "ent_anim_cig_exhale_mth_car";
static constexpr char* ANM_CIG_EXHALE_NSE_CAR = "ent_anim_cig_exhale_nse_car";

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

void cSmokingSequence::StopAllPTFX()
{
	StopPTFX(&cigarettePTFXHandle);
	StopPTFX(&hasExhaledNose);
	REMOVE_NAMED_PTFX_ASSET(CORE_PTFX_ASSET);
	return;
}

void cSmokingSequence::PlayPTFX()
{
	if (sequenceState == STREAM_ASSETS_IN || sequenceState == FLUSH_ASSETS || sequenceState == FINISHED)
	{
		StopAllPTFX();
		return;
	}

	if (!HAS_NAMED_PTFX_ASSET_LOADED(CORE_PTFX_ASSET))
		REQUEST_NAMED_PTFX_ASSET(CORE_PTFX_ASSET);

	// offsets can be found inside scenarios
	const float enterAnimTime = GET_ENTITY_ANIM_CURRENT_TIME(playerPed, smokeEnterAnimDict, smokeEnterAnim);
	if (enterAnimTime >= 0.4826087f && cigarettePTFXHandle == NULL)
	{
		USE_PARTICLE_FX_ASSET(CORE_PTFX_ASSET);
		cigarettePTFXHandle = START_PARTICLE_FX_LOOPED_ON_PED_BONE(ANM_CIG_SMOKE, playerPed, -0.08f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, BONETAG_PH_R_HAND, 1.0f, false, false, false);
	}

	const float baseAnimTime = GET_ENTITY_ANIM_CURRENT_TIME(playerPed, smokeBaseAnimDict, smokeBaseAnim);
	if (baseAnimTime < 0.7965517f)
		hasExhaledNose = NULL;

	if (baseAnimTime >= 0.7965517f && hasExhaledNose == NULL)
	{
		USE_PARTICLE_FX_ASSET(CORE_PTFX_ASSET);
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
		shouldStopSequence = false;
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

	if (state == EXITING && sequenceState != EXITING && sequenceState != FLUSH_ASSETS && sequenceState != FINISHED)
		sequenceState = EXITING;
	else if (state != EXITING)
		sequenceState = state;

	return;
}

void cSmokingSequence::UpdateControls() 
{
	if (sequenceState == EXITING || sequenceState == FLUSH_ASSETS || sequenceState == FINISHED)
		return;

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
		shouldStopSequence = true;

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
		if (shouldStopSequence)
		{
			if (stopTimer.Get() > maxStopTimer)
			{
				ForceStop();
				return;
			}

			SetState(EXITING);
		}
		else
			stopTimer.Set(0);

		UpdateControls();
		return;
	}

	shouldStopSequence = false; //Reset var
	DeleteObject(&item); //Force delete old item
	return;
}

//////////////////////////////////DRINKING//////////////////////////////////
static constexpr int beerHash = 0x28BE7556;		//Prop_AMB_Beer_Bottle
static constexpr char* drinkingAnimDict = "mp_player_intdrink";
static constexpr char* drinkingEnterAnim = "intro_bottle";
static constexpr char* drinkingBaseAnim = "loop_bottle";
static constexpr char* drinkingExitAnim = "outro_bottle";

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
		shouldStopSequence = false;
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

	if (state == EXITING && sequenceState != EXITING && sequenceState != FLUSH_ASSETS && sequenceState != FINISHED)
		sequenceState = EXITING;
	else if (state != EXITING)
		sequenceState = state;

	return;
}

void cDrinkingSequence::UpdateControls()
{
	if (sequenceState == EXITING || sequenceState == FLUSH_ASSETS || sequenceState == FINISHED)
		return;

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
		shouldStopSequence = true;

	return;
}

void cDrinkingSequence::ForceStop()
{
	if (sequenceState == FLUSH_ASSETS || sequenceState == FINISHED)
		return;

	StopAllAnims();
	shouldStopSequence = false;
	sequenceState = FLUSH_ASSETS;
	PlaySequence();
	DeleteObject(&item);
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
		if (shouldStopSequence)
		{
			if (stopTimer.Get() > maxStopTimer)
			{
				ForceStop();
				return;
			}

			SetState(EXITING);
		}
		else
			stopTimer.Set(0);

		UpdateControls();
		return;
	}

	shouldStopSequence = false; //Reset var
	return;
}

//////////////////////////////////LEAF BLOWER//////////////////////////////////
static constexpr int leafBlowerHash = 0x5F989485;		//Prop_Leaf_Blower_01
static constexpr char* leafBlowerClipSet = "move_m@leaf_blower";
static constexpr char* ANM_LEAF_BLOWER = "ent_anim_leaf_blower";
static constexpr char* leafBlowerAudioBank = "SCRIPT/GARDEN_LEAF_BLOWER";
static constexpr char* leafBlowerSound = "GARDENING_LEAFBLOWER_ANIM_TRIGGERED";

void cLeafBlowerSequence::StopAllPTFXAndSounds()
{
	StopPTFX(&leafBlowerPTFXHandle);
	REMOVE_NAMED_PTFX_ASSET(CORE_PTFX_ASSET);
	STOP_SOUND(soundID);
	RELEASE_SOUND_ID(soundID);
	return;
}

void cLeafBlowerSequence::PlayPTFXAndSound()
{
	if (!isUsingLeafBlower || sequenceState == STREAM_ASSETS_IN || sequenceState == FLUSH_ASSETS || sequenceState == FINISHED)
	{
		StopAllPTFXAndSounds();
		return;
	}

	if (!HAS_NAMED_PTFX_ASSET_LOADED(CORE_PTFX_ASSET))
		REQUEST_NAMED_PTFX_ASSET(CORE_PTFX_ASSET);

	if (!DOES_PARTICLE_FX_LOOPED_EXIST(leafBlowerPTFXHandle) && isUsingLeafBlower)
	{
		USE_PARTICLE_FX_ASSET(CORE_PTFX_ASSET);
		leafBlowerPTFXHandle = START_PARTICLE_FX_LOOPED_ON_PED_BONE(ANM_LEAF_BLOWER, playerPed, 0.9f, 0.0f, -0.25f, 0.0f, 0.0f, 0.0f, BONETAG_PH_R_HAND, 1.0f, false, false, false);
		soundID = GET_SOUND_ID();
		PLAY_SOUND_FROM_ENTITY(soundID, leafBlowerSound, item, NULL, false, 0);
	}
	return;
}

void cLeafBlowerSequence::PlaySequence()
{
	const int rightHandID = GET_PED_BONE_INDEX(playerPed, BONETAG_PH_R_HAND);

	SetPlayerControls(); //Player control should be disabled here and not during the sequence

	switch (sequenceState)
	{
	case INITIALIZED:
		item = CreateObject(leafBlowerHash);
		SET_ENTITY_AS_MISSION_ENTITY(item, true, true);

		if (!IS_ENTITY_ATTACHED(item))
			ATTACH_ENTITY_TO_ENTITY(item, playerPed, rightHandID, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, true, true, false, false, 2, true);

		SET_PED_WEAPON_MOVEMENT_CLIPSET(playerPed, leafBlowerClipSet);
		TASK_LOOK_AT_ENTITY(playerPed, item, -1, SLF_SLOW_TURN_RATE, 2);
		sequenceState = LOOP;
		break;
	case WAITING_FOR_ANIMATION_TO_END:
		if (!IS_ENTITY_PLAYING_ANIM(playerPed, lastAnimDict, lastAnim, 3))
			sequenceState = nextSequenceState;
		break;
	case LOOP:
		break;
	case EXITING:
		RESET_PED_WEAPON_MOVEMENT_CLIPSET(playerPed);
		TASK_CLEAR_LOOK_AT(playerPed);
		sequenceState = FLUSH_ASSETS;
		break;
	case FLUSH_ASSETS:
		DETACH_ENTITY(item, false, false);
		SET_ENTITY_AS_NO_LONGER_NEEDED(&item);
		SET_MODEL_AS_NO_LONGER_NEEDED(leafBlowerHash);
		REMOVE_CLIP_SET(leafBlowerClipSet);
		STOP_SOUND(soundID);
		RELEASE_SOUND_ID(soundID);
		RELEASE_NAMED_SCRIPT_AUDIO_BANK(leafBlowerAudioBank);
		shouldStopSequence = false;
		sequenceState = FINISHED;
		break;
	case STREAM_ASSETS_IN:
		if (RequestModel(leafBlowerHash) && RequestClipSet(leafBlowerClipSet) && RequestAudioBank(leafBlowerAudioBank))
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
	PlayPTFXAndSound();

	//Disable ped gestures and block non-player peds from reacting to temporary events
	SetPedMovementAndReactions();
	return;
}

void cLeafBlowerSequence::SetState(int state)
{
	if (state == EXITING && sequenceState != EXITING && sequenceState != FLUSH_ASSETS && sequenceState != FINISHED)
		sequenceState = EXITING;
	else if (state != EXITING)
		sequenceState = state;

	return;
}

void cLeafBlowerSequence::UpdateControls()
{
	if (sequenceState == EXITING || sequenceState == FLUSH_ASSETS || sequenceState == FINISHED)
		return;

	AddScaleformInstructionalButton(control, input, "Toggle (hold to stop)", true);
	RunScaleformInstructionalButtons();

	if (IS_DISABLED_CONTROL_JUST_PRESSED(control, input))
	{
		controlTimer.Set(0);
		return;
	}

	if (controlTimer.Get() < holdTime)
	{
		if (IS_DISABLED_CONTROL_JUST_RELEASED(control, input))
			isUsingLeafBlower ^= true;
	}
	else if (IS_DISABLED_CONTROL_PRESSED(control, input))
		shouldStopSequence = true;

	return;
}

void cLeafBlowerSequence::ForceStop()
{
	if (sequenceState == FLUSH_ASSETS || sequenceState == FINISHED)
		return;

	RESET_PED_WEAPON_MOVEMENT_CLIPSET(playerPed);
	TASK_CLEAR_LOOK_AT(playerPed);
	StopAllPTFXAndSounds();
	shouldStopSequence = false;
	sequenceState = FLUSH_ASSETS;
	PlaySequence();
	DeleteObject(&item);
	return;
}

void cLeafBlowerSequence::Update()
{
	if (sequenceState != FINISHED)
	{
		if (!AdditionalChecks(playerPed))
		{
			ForceStop();
			return;
		}

		PlaySequence();
		if (shouldStopSequence)
		{
			if (stopTimer.Get() > maxStopTimer)
			{
				ForceStop();
				return;
			}

			SetState(EXITING);
		}	
		else
			stopTimer.Set(0);

		UpdateControls();
		return;
	}

	shouldStopSequence = false; //Reset var
	return;
}

//////////////////////////////////JOG//////////////////////////////////
static constexpr char* joggerAnimDict = "move_m@jogger";
static constexpr char* joggerIdleAnim = "idle";
static constexpr char* joggerWalkAnim = "run";

void cJogSequence::PlaySequence()
{
	SetPlayerControls(); //Player control should be disabled here and not during the sequence

	switch (sequenceState)
	{
	case INITIALIZED:
		SET_PED_ALTERNATE_MOVEMENT_ANIM(playerPed, AAT_IDLE, joggerAnimDict, joggerIdleAnim, 8.0f, true);
		SET_PED_ALTERNATE_MOVEMENT_ANIM(playerPed, AAT_WALK, joggerAnimDict, joggerWalkAnim, 8.0f, true);
		sequenceState = LOOP;
		break;
	case WAITING_FOR_ANIMATION_TO_END:
		if (!IS_ENTITY_PLAYING_ANIM(playerPed, lastAnimDict, lastAnim, 3))
			sequenceState = nextSequenceState;
		break;
	case LOOP:
		break;
	case EXITING:
		CLEAR_PED_ALTERNATE_MOVEMENT_ANIM(playerPed, AAT_IDLE, 8.0f);
		CLEAR_PED_ALTERNATE_MOVEMENT_ANIM(playerPed, AAT_WALK, 8.0f);
		sequenceState = FLUSH_ASSETS;
		break;
	case FLUSH_ASSETS:
		REMOVE_ANIM_DICT(joggerAnimDict);
		shouldStopSequence = false;
		sequenceState = FINISHED;
		break;
	case STREAM_ASSETS_IN:
		if (RequestAnimDict(joggerAnimDict))
		{
			sequenceState = INITIALIZED;
			nextSequenceState = NULL;
			shouldPlayerStandStill = false;
			lastAnimDict = NULL;
			lastAnim = NULL;
		}
		break;
	}

	//Disable ped gestures and block non-player peds from reacting to temporary events
	SetPedMovementAndReactions();
	return;
}

void cJogSequence::SetState(int state)
{
	if (state == EXITING && sequenceState != EXITING && sequenceState != FLUSH_ASSETS && sequenceState != FINISHED)
		sequenceState = EXITING;
	else if (state != EXITING)
		sequenceState = state;

	return;
}

void cJogSequence::UpdateControls()
{
	if (sequenceState == EXITING || sequenceState == FLUSH_ASSETS || sequenceState == FINISHED)
		return;

	AddScaleformInstructionalButton(control, input, "Hold to stop", true);
	RunScaleformInstructionalButtons();

	if (IS_DISABLED_CONTROL_JUST_PRESSED(control, input))
	{
		controlTimer.Set(0);
		return;
	}

	if (controlTimer.Get() > holdTime && IS_DISABLED_CONTROL_PRESSED(control, input))
		shouldStopSequence = true;

	return;
}

void cJogSequence::ForceStop()
{
	if (sequenceState == FLUSH_ASSETS || sequenceState == FINISHED)
		return;

	CLEAR_PED_ALTERNATE_MOVEMENT_ANIM(playerPed, AAT_IDLE, 8.0f);
	CLEAR_PED_ALTERNATE_MOVEMENT_ANIM(playerPed, AAT_WALK, 8.0f);
	shouldStopSequence = false;
	sequenceState = FLUSH_ASSETS;
	PlaySequence();
	return;
}

void cJogSequence::Update()
{
	if (sequenceState != FINISHED)
	{
		if (!AdditionalChecks(playerPed))
		{
			ForceStop();
			return;
		}

		PlaySequence();
		if (shouldStopSequence)
		{
			if (stopTimer.Get() > maxStopTimer)
			{
				ForceStop();
				return;
			}

			SetState(EXITING);
		}
		else
			stopTimer.Set(0);

		UpdateControls();
		return;
	}

	shouldStopSequence = false; //Reset var
	return;
}