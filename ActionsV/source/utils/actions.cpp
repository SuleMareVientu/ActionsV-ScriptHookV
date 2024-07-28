//ScriptHook
#include <natives.h>
// #include <types.h> //Already included in globals.h
//Custom
#include "actions.h"
#include "functions.h"
#include "keyboard.h"
#include "..\script.h"

cSmokingSequence smokingSequence;
cDrinkingSequence drinkingSequence;
cLeafBlowerSequence leafBlowerSequence;
cJogSequence jogSequence;
cClipboardSequence clipboardSequence;
cGuitarSequence guitarSequence;
cBongosSequence bongosSequence;
cMopSequence mopSequence;
cMopWithBucketSequence mopWithBucketSequence;
cCameraSequence cameraSequence;

static constexpr int maxSequences = 10;
cSequence* sequences[maxSequences] = { &smokingSequence, &drinkingSequence, &leafBlowerSequence, &jogSequence, &clipboardSequence, 
									   &guitarSequence, &bongosSequence, &mopSequence, &mopWithBucketSequence, &cameraSequence };

static constexpr char* CORE_PTFX_ASSET = "core";

void UpdateSequences()
{
	LOOP(i, maxSequences)
	{ sequences[i]->Update(); }
	return;
}

static bool NoSequenceIsActive()
{
	LOOP(i, maxSequences)
	{
		if (sequences[i]->IsActive())
			return false;
	}

	return true;
}

static void StopActiveSequence()
{
	LOOP(i, maxSequences)
	{
		if (sequences[i]->IsActive())
			sequences[i]->Stop();
	}

	return;
}

#pragma region Sequence
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

void cSequence::PlayAnimAndWait(char *animDict, char *anim, int flag, int nextState, float startPhase, float blendInSpeed, float blendOutSpeed, bool standStill, int duration)
{
	PlayAnimTask(playerPed, animDict, anim, flag, duration, blendInSpeed, blendOutSpeed, startPhase);
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
		DisablePlayerActionsThisFrame(canSprint, maxMoveBlendRatio);

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
		EnablePedResetFlag(playerPed, PRF_DisableSecondaryAnimationTasks);
		SET_PED_CAN_PLAY_GESTURE_ANIMS(playerPed, false);
		SET_PED_CAN_PLAY_AMBIENT_ANIMS(playerPed, false);
		SET_PED_CAN_PLAY_AMBIENT_BASE_ANIMS(playerPed, false);
		SET_PED_CAN_PLAY_AMBIENT_IDLES(playerPed, true, true);
		SET_PED_STEALTH_MOVEMENT(playerPed, false, NULL);
		SET_PED_USING_ACTION_MODE(playerPed, false, -1, NULL);
	}
	return;
}
#pragma endregion

//////////////////////////////////SMOKING//////////////////////////////
#pragma region Smoking
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
	StopAnimTask(playerPed, smokeEnterAnimDict, smokeEnterAnim);
	StopAnimTask(playerPed, smokeBaseAnimDict, smokeBaseAnim);
	StopAnimTask(playerPed, smokeExitAnimDict, smokeExitAnim);
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

	if (!RequestPTFXAsset(CORE_PTFX_ASSET))
		return;

	// offsets can be found inside scenarios
	const float enterAnimTime = GET_ENTITY_ANIM_CURRENT_TIME(playerPed, smokeEnterAnimDict, smokeEnterAnim);
	if (enterAnimTime >= 0.4826087f && cigarettePTFXHandle == NULL)
		cigarettePTFXHandle = StartParticleFxLoopedOnPedBone(CORE_PTFX_ASSET, ANM_CIG_SMOKE, playerPed, BONETAG_PH_R_HAND, 1.0f, -0.08f);

	const float baseAnimTime = GET_ENTITY_ANIM_CURRENT_TIME(playerPed, smokeBaseAnimDict, smokeBaseAnim);
	if (baseAnimTime < 0.7965517f)
		hasExhaledNose = NULL;

	if (baseAnimTime >= 0.7965517f && hasExhaledNose == NULL)
		hasExhaledNose = StartParticleFxNonLoopedOnPedBone(CORE_PTFX_ASSET, ANM_CIG_EXHALE_NSE, playerPed, BONETAG_HEAD, 1.0f, 0.02f, 0.16f);

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

	StopAnimTask(playerPed, animDict, anim);

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
#pragma endregion

//////////////////////////////////DRINKING/////////////////////////////
#pragma region Drinking
static constexpr int beerHash = 0x28BE7556;		//Prop_AMB_Beer_Bottle
static constexpr char* drinkingAnimDict = "mp_player_intdrink";
static constexpr char* drinkingEnterAnim = "intro_bottle";
static constexpr char* drinkingBaseAnim = "loop_bottle";
static constexpr char* drinkingExitAnim = "outro_bottle";

void cDrinkingSequence::StopAllAnims()
{
	StopAnimTask(playerPed, drinkingAnimDict, drinkingEnterAnim);
	StopAnimTask(playerPed, drinkingAnimDict, drinkingBaseAnim);
	StopAnimTask(playerPed, drinkingAnimDict, drinkingExitAnim);
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

	StopAnimTask(playerPed, animDict, anim);

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
#pragma endregion

//////////////////////////////////LEAF BLOWER//////////////////////////
#pragma region LeafBlower
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

	if (!RequestPTFXAsset(CORE_PTFX_ASSET))
		return;

	if (!DOES_PARTICLE_FX_LOOPED_EXIST(leafBlowerPTFXHandle) && isUsingLeafBlower)
	{
		leafBlowerPTFXHandle = StartParticleFxLoopedOnPedBone(CORE_PTFX_ASSET, ANM_LEAF_BLOWER, playerPed, BONETAG_PH_R_HAND, 1.0f, 0.9f, 0.0f, -0.25f);
		soundID = GET_SOUND_ID();
		PLAY_SOUND_FROM_ENTITY(soundID, leafBlowerSound, playerPed, NULL, false, 0);
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
#pragma endregion

//////////////////////////////////JOG//////////////////////////////////
#pragma region Jog
//static constexpr char* joggerClipSet = "hash_813d2ab0_arqvdozd_collision";	//move_m@jogger		amb@world_human_jog_male@base		hash_813d2ab0_arqvdozd_collision
static constexpr char* joggerAnimDict = "move_m@jogger";
static constexpr char* joggerIdleAnim = "idle";
static constexpr char* joggerRunAnim = "run";

void cJogSequence::PlaySequence()
{
	SetPlayerControls(); //Player control should be disabled here and not during the sequence

	switch (sequenceState)
	{
	case INITIALIZED:
		SET_PED_ALTERNATE_MOVEMENT_ANIM(playerPed, AAT_IDLE, joggerAnimDict, joggerIdleAnim, 8.0f, true);
		SET_PED_ALTERNATE_MOVEMENT_ANIM(playerPed, AAT_WALK, joggerAnimDict, joggerRunAnim, 8.0f, true);
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
#pragma endregion

//////////////////////////////////CLIPBOARD//////////////////////////
#pragma region Clipboard
static constexpr int clipboardHash = 0x3D387228;		//P_CS_Clipboard
static constexpr char* clipboardClipSet = "move_m@clipboard";

void cClipboardSequence::PlaySequence()
{
	const int leftHandID = GET_PED_BONE_INDEX(playerPed, BONETAG_PH_L_HAND);

	SetPlayerControls(); //Player control should be disabled here and not during the sequence

	switch (sequenceState)
	{
	case INITIALIZED:
		item = CreateObject(clipboardHash);
		SET_ENTITY_AS_MISSION_ENTITY(item, true, true);
		ATTACH_ENTITY_TO_ENTITY(item, playerPed, leftHandID, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, true, true, false, false, 2, true);
		SET_PED_WEAPON_MOVEMENT_CLIPSET(playerPed, clipboardClipSet);
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
		SET_MODEL_AS_NO_LONGER_NEEDED(clipboardHash);
		REMOVE_CLIP_SET(clipboardClipSet);
		shouldStopSequence = false;
		sequenceState = FINISHED;
		break;
	case STREAM_ASSETS_IN:
		if (RequestModel(clipboardHash) && RequestClipSet(clipboardClipSet))
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

void cClipboardSequence::SetState(int state)
{
	if (state == EXITING && sequenceState != EXITING && sequenceState != FLUSH_ASSETS && sequenceState != FINISHED)
		sequenceState = EXITING;
	else if (state != EXITING)
		sequenceState = state;

	return;
}

void cClipboardSequence::UpdateControls()
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

void cClipboardSequence::ForceStop()
{
	if (sequenceState == FLUSH_ASSETS || sequenceState == FINISHED)
		return;

	RESET_PED_WEAPON_MOVEMENT_CLIPSET(playerPed);
	TASK_CLEAR_LOOK_AT(playerPed);
	shouldStopSequence = false;
	sequenceState = FLUSH_ASSETS;
	PlaySequence();
	DeleteObject(&item);
	return;
}

void cClipboardSequence::Update()
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
#pragma endregion

//////////////////////////////////GUITAR/////////////////////////////
#pragma region Guitar
static constexpr int guitarHash = 0xD5C0BC07;		//Prop_ACC_Guitar_01
static constexpr char* guitarAnimDict = "amb@world_human_musician@guitar@male@base";
static constexpr char* guitarBaseAnim = "base";
//static constexpr char* guitarAudioBank = "STREAMS/AMBIENT_MUSICIAN_GUITAR";
static constexpr char* guitarStream = "hash_80dc636b_biknqys_collision";

void cGuitarSequence::StopAllAnims()
{
	StopAnimTask(playerPed, guitarAnimDict, guitarBaseAnim);
	REMOVE_ANIM_DICT(guitarAnimDict);
	return;
}

void cGuitarSequence::PlaySequence()
{
	const int leftHandID = GET_PED_BONE_INDEX(playerPed, BONETAG_PH_L_HAND);

	SetPlayerControls(); //Player control should be disabled here and not during the sequence

	switch (sequenceState)
	{
	case INITIALIZED:
		StopAudioStream();
		item = CreateObject(guitarHash);
		SET_ENTITY_AS_MISSION_ENTITY(item, true, true);
		ATTACH_ENTITY_TO_ENTITY(item, playerPed, leftHandID, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, true, true, false, false, 2, true);
		PlayAnimTask(playerPed, guitarAnimDict, guitarBaseAnim, upperSecondaryAF | AF_LOOPING);
		sequenceState = LOOP;
		break;
	case WAITING_FOR_ANIMATION_TO_END:
		if (!IS_ENTITY_PLAYING_ANIM(playerPed, lastAnimDict, lastAnim, 3))
			sequenceState = nextSequenceState;
		break;
	case LOOP:
		if (!IS_STREAM_PLAYING())
		{
			if (LOAD_STREAM(guitarStream, NULL))
				PLAY_STREAM_FROM_PED(playerPed);
		}
		break;
	case EXITING:
		StopAllAnims();
		StopAudioStream();
		sequenceState = FLUSH_ASSETS;
		break;
	case FLUSH_ASSETS:
		REMOVE_ANIM_DICT(guitarAnimDict);
		DETACH_ENTITY(item, false, false);
		SET_ENTITY_AS_NO_LONGER_NEEDED(&item);
		SET_MODEL_AS_NO_LONGER_NEEDED(guitarHash);
		shouldStopSequence = false;
		sequenceState = FINISHED;
		break;
	case STREAM_ASSETS_IN:
		if (RequestModel(guitarHash) && RequestAnimDict(guitarAnimDict))
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

void cGuitarSequence::SetState(int state)
{
	StopAnimTask(playerPed, guitarAnimDict, guitarBaseAnim);

	if (state == EXITING && sequenceState != EXITING && sequenceState != FLUSH_ASSETS && sequenceState != FINISHED)
		sequenceState = EXITING;
	else if (state != EXITING)
		sequenceState = state;

	return;
}

void cGuitarSequence::UpdateControls()
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

void cGuitarSequence::ForceStop()
{
	if (sequenceState == FLUSH_ASSETS || sequenceState == FINISHED)
		return;

	StopAllAnims();
	StopAudioStream();
	shouldStopSequence = false;
	sequenceState = FLUSH_ASSETS;
	PlaySequence();
	DeleteObject(&item);
	return;
}

void cGuitarSequence::Update()
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
#pragma endregion

//////////////////////////////////BONGOS/////////////////////////////
#pragma region Bongos
static constexpr int bongosHash = 0x2347ED83;		//Prop_Bongos_01
static constexpr char* bongosAnimDict = "amb@world_human_musician@bongos@male@base";
static constexpr char* bongosBaseAnim = "base";
static constexpr char* bongosStream = "hash_a38bad80_zvulbdw_collision";

void cBongosSequence::StopAllAnims()
{
	StopAnimTask(playerPed, bongosAnimDict, bongosBaseAnim);
	REMOVE_ANIM_DICT(bongosAnimDict);
	return;
}

void cBongosSequence::PlaySequence()
{
	const int leftHandID = GET_PED_BONE_INDEX(playerPed, BONETAG_PH_L_HAND);

	SetPlayerControls(); //Player control should be disabled here and not during the sequence

	switch (sequenceState)
	{
	case INITIALIZED:
		StopAudioStream();
		item = CreateObject(bongosHash);
		SET_ENTITY_AS_MISSION_ENTITY(item, true, true);
		ATTACH_ENTITY_TO_ENTITY(item, playerPed, leftHandID, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, true, true, false, false, 2, true);
		PlayAnimTask(playerPed, bongosAnimDict, bongosBaseAnim, upperSecondaryAF | AF_LOOPING);
		sequenceState = LOOP;
		break;
	case WAITING_FOR_ANIMATION_TO_END:
		if (!IS_ENTITY_PLAYING_ANIM(playerPed, lastAnimDict, lastAnim, 3))
			sequenceState = nextSequenceState;
		break;
	case LOOP:
		if (!IS_STREAM_PLAYING())
		{
			if (LOAD_STREAM(bongosStream, NULL))
				PLAY_STREAM_FROM_PED(playerPed);
		}
		break;
	case EXITING:
		StopAllAnims();
		StopAudioStream();
		sequenceState = FLUSH_ASSETS;
		break;
	case FLUSH_ASSETS:
		REMOVE_ANIM_DICT(bongosAnimDict);
		DETACH_ENTITY(item, false, false);
		SET_ENTITY_AS_NO_LONGER_NEEDED(&item);
		SET_MODEL_AS_NO_LONGER_NEEDED(bongosHash);
		shouldStopSequence = false;
		sequenceState = FINISHED;
		break;
	case STREAM_ASSETS_IN:
		if (RequestModel(bongosHash) && RequestAnimDict(bongosAnimDict))
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

void cBongosSequence::SetState(int state)
{
	StopAnimTask(playerPed, bongosAnimDict, bongosBaseAnim);

	if (state == EXITING && sequenceState != EXITING && sequenceState != FLUSH_ASSETS && sequenceState != FINISHED)
		sequenceState = EXITING;
	else if (state != EXITING)
		sequenceState = state;

	return;
}

void cBongosSequence::UpdateControls()
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

void cBongosSequence::ForceStop()
{
	if (sequenceState == FLUSH_ASSETS || sequenceState == FINISHED)
		return;

	StopAllAnims();
	StopAudioStream();
	shouldStopSequence = false;
	sequenceState = FLUSH_ASSETS;
	PlaySequence();
	DeleteObject(&item);
	return;
}

void cBongosSequence::Update()
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
#pragma endregion

//////////////////////////////////MOP//////////////////////////
#pragma region Mop
static constexpr int mopHash = 0xECE03F63;		//Prop_CS_Mop_S
static constexpr char* mopClipSet = "move_ped_wpn_mop";

void cMopSequence::PlaySequence()
{
	const int leftHandID = GET_PED_BONE_INDEX(playerPed, BONETAG_PH_L_HAND);

	SetPlayerControls(); //Player control should be disabled here and not during the sequence

	switch (sequenceState)
	{
	case INITIALIZED:
		item = CreateObject(mopHash);
		SET_ENTITY_AS_MISSION_ENTITY(item, true, true);
		ATTACH_ENTITY_TO_ENTITY(item, playerPed, leftHandID, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, true, true, false, false, 2, true);
		SET_PED_WEAPON_MOVEMENT_CLIPSET(playerPed, mopClipSet);
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
		sequenceState = FLUSH_ASSETS;
		break;
	case FLUSH_ASSETS:
		DETACH_ENTITY(item, false, false);
		SET_ENTITY_AS_NO_LONGER_NEEDED(&item);
		SET_MODEL_AS_NO_LONGER_NEEDED(mopHash);
		REMOVE_CLIP_SET(mopClipSet);
		shouldStopSequence = false;
		sequenceState = FINISHED;
		break;
	case STREAM_ASSETS_IN:
		if (RequestModel(mopHash) && RequestClipSet(mopClipSet))
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

void cMopSequence::SetState(int state)
{
	if (state == EXITING && sequenceState != EXITING && sequenceState != FLUSH_ASSETS && sequenceState != FINISHED)
		sequenceState = EXITING;
	else if (state != EXITING)
		sequenceState = state;

	return;
}

void cMopSequence::UpdateControls()
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

void cMopSequence::ForceStop()
{
	if (sequenceState == FLUSH_ASSETS || sequenceState == FINISHED)
		return;

	RESET_PED_WEAPON_MOVEMENT_CLIPSET(playerPed);
	shouldStopSequence = false;
	sequenceState = FLUSH_ASSETS;
	PlaySequence();
	DeleteObject(&item);
	return;
}

void cMopSequence::Update()
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
#pragma endregion

//////////////////////////////////MOP WITH BUCKET//////////////////////////
#pragma region MopWithBucket
static constexpr int bucketHash = 0x210974D7;		//Prop_CS_Bucket_S
static constexpr char* bucketClipSet = "move_ped_wpn_bucket";
static constexpr char* bucketAnimDict = "missfbi_s4mop";
static constexpr char* mopIdleAnim = "mop_idle";
static constexpr char* bucketIdleAnim = "bucket_idle";

void cMopWithBucketSequence::PlaySequence()
{
	const int rightHandID = GET_PED_BONE_INDEX(playerPed, BONETAG_PH_R_HAND);
	const int leftHandID = GET_PED_BONE_INDEX(playerPed, BONETAG_PH_L_HAND);

	SetPlayerControls(); //Player control should be disabled here and not during the sequence

	switch (sequenceState)
	{
	case INITIALIZED:
		bucket = CreateObject(bucketHash);
		SET_ENTITY_AS_MISSION_ENTITY(bucket, true, true);
		ATTACH_ENTITY_TO_ENTITY(bucket, playerPed, rightHandID, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, true, true, false, false, 2, true);
		PLAY_ENTITY_ANIM(bucket, bucketIdleAnim, bucketAnimDict, INSTANT_BLEND_IN, true, false, false, 0.0f, NULL);

		mop = CreateObject(mopHash);
		SET_ENTITY_AS_MISSION_ENTITY(mop, true, true);
		ATTACH_ENTITY_TO_ENTITY(mop, playerPed, leftHandID, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, true, true, false, false, 2, true);
		PLAY_ENTITY_ANIM(mop, mopIdleAnim, bucketAnimDict, INSTANT_BLEND_IN, true, false, false, 0.0f, NULL);

		SET_PED_WEAPON_MOVEMENT_CLIPSET(playerPed, bucketClipSet);
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
		sequenceState = FLUSH_ASSETS;
		break;
	case FLUSH_ASSETS:
		DETACH_ENTITY(bucket, false, false);
		SET_ENTITY_AS_NO_LONGER_NEEDED(&bucket);
		SET_MODEL_AS_NO_LONGER_NEEDED(bucketHash);

		DETACH_ENTITY(mop, false, false);
		SET_ENTITY_AS_NO_LONGER_NEEDED(&mop);
		SET_MODEL_AS_NO_LONGER_NEEDED(mopHash);

		REMOVE_ANIM_DICT(bucketAnimDict);
		REMOVE_CLIP_SET(bucketClipSet);
		shouldStopSequence = false;
		sequenceState = FINISHED;
		break;
	case STREAM_ASSETS_IN:
		if (RequestModel(bucketHash) && RequestModel(mopHash) && RequestAnimDict(bucketAnimDict) && RequestClipSet(bucketClipSet))
		{
			sequenceState = INITIALIZED;
			nextSequenceState = NULL;
			shouldPlayerStandStill = false;
			lastAnimDict = NULL;
			lastAnim = NULL;
			mop = NULL;
			bucket = NULL;
		}
		break;
	}

	//Disable ped gestures and block non-player peds from reacting to temporary events
	SetPedMovementAndReactions();
	return;
}

void cMopWithBucketSequence::SetState(int state)
{
	if (state == EXITING && sequenceState != EXITING && sequenceState != FLUSH_ASSETS && sequenceState != FINISHED)
		sequenceState = EXITING;
	else if (state != EXITING)
		sequenceState = state;

	return;
}

void cMopWithBucketSequence::UpdateControls()
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

void cMopWithBucketSequence::ForceStop()
{
	if (sequenceState == FLUSH_ASSETS || sequenceState == FINISHED)
		return;

	RESET_PED_WEAPON_MOVEMENT_CLIPSET(playerPed);
	shouldStopSequence = false;
	sequenceState = FLUSH_ASSETS;
	PlaySequence();
	DeleteObject(&bucket);
	DeleteObject(&mop);
	return;
}

void cMopWithBucketSequence::Update()
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
#pragma endregion

//////////////////////////////////CAMERA//////////////////////////
#pragma region Camera
static constexpr int cameraHash = 0x288DC72A;		//Prop_Pap_Camera_01
static constexpr int flashUnitHash = 0xF3584679;	//Prop_Flash_Unit
static constexpr char* cameraClipSet = "random@escape_paparazzi@standing@";
static constexpr char* cameraActionAnim = "idle_a";
static constexpr char* ANM_PAPARAZZI_FLASH = "ent_anim_paparazzi_flash";
static constexpr char* cameraAudioBank = "SCRIPT/Distant_Camera_Flash";
static constexpr char* cameraSound = "GENERIC_CAMERA_FLASH";

void cCameraSequence::PlayPTFXAndSound()
{
	if (!RequestPTFXAsset(CORE_PTFX_ASSET))
		return;

	if (playFlashSound && GET_ENTITY_ANIM_CURRENT_TIME(playerPed, cameraClipSet, cameraActionAnim) > 0.675f)
	{	
		StartParticleFxNonLoopedOnEntity(CORE_PTFX_ASSET, ANM_PAPARAZZI_FLASH, flashUnit, 1.0f, -0.04f, -0.06f);
		PLAY_SOUND_FROM_ENTITY(-1, cameraSound, playerPed, NULL, false, 0);
		playFlashSound = false;
	}
	return;
}

void cCameraSequence::PlaySequence()
{
	const int rightHandID = GET_PED_BONE_INDEX(playerPed, BONETAG_PH_R_HAND);

	SetPlayerControls(); //Player control should be disabled here and not during the sequence

	PrintFloat(GET_ENTITY_ANIM_CURRENT_TIME(playerPed, cameraClipSet, cameraActionAnim));

	switch (sequenceState)
	{
	case INITIALIZED:
		camera = CreateObject(cameraHash);
		SET_ENTITY_AS_MISSION_ENTITY(camera, true, true);
		ATTACH_ENTITY_TO_ENTITY(camera, playerPed, rightHandID, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, true, true, false, false, 2, true);

		flashUnit = CreateObject(flashUnitHash);
		SET_ENTITY_AS_MISSION_ENTITY(flashUnit, true, true);
		ATTACH_ENTITY_TO_ENTITY(flashUnit, camera, NULL, 0.107f, 0.025f, 0.177f, 0.0f, 0.0f, 0.0f, true, true, false, false, 2, true);

		SET_PED_WEAPON_MOVEMENT_CLIPSET(playerPed, cameraClipSet);
		sequenceState = LOOP;
		break;
	case WAITING_FOR_ANIMATION_TO_END:
		if (!IS_ENTITY_PLAYING_ANIM(playerPed, lastAnimDict, lastAnim, 3))
			sequenceState = nextSequenceState;
		break;
	case LOOP:
		break;
	case TAKE_PHOTO:
		if (!IS_ENTITY_PLAYING_ANIM(playerPed, cameraClipSet, cameraActionAnim, 3))
		{
			PlayAnimAndWait(cameraClipSet, cameraActionAnim, upperSecondaryAF, LOOP);
			playFlashSound = true;
		}
		break;
	case EXITING:
		RESET_PED_WEAPON_MOVEMENT_CLIPSET(playerPed);
		sequenceState = FLUSH_ASSETS;
		break;
	case FLUSH_ASSETS:
		DETACH_ENTITY(camera, false, false);
		SET_ENTITY_AS_NO_LONGER_NEEDED(&camera);
		SET_MODEL_AS_NO_LONGER_NEEDED(cameraHash);

		SET_ENTITY_AS_NO_LONGER_NEEDED(&flashUnit);
		SET_MODEL_AS_NO_LONGER_NEEDED(flashUnitHash);
		
		REMOVE_CLIP_SET(cameraClipSet);
		REMOVE_ANIM_DICT(cameraClipSet);
		RELEASE_NAMED_SCRIPT_AUDIO_BANK(cameraAudioBank);
		REMOVE_NAMED_PTFX_ASSET(CORE_PTFX_ASSET);
		shouldStopSequence = false;
		sequenceState = FINISHED;
		break;
	case STREAM_ASSETS_IN:
		if (RequestModel(cameraHash) && RequestModel(flashUnitHash) && RequestClipSet(cameraClipSet) && RequestAnimDict(cameraClipSet) && RequestAudioBank(cameraAudioBank))
		{
			sequenceState = INITIALIZED;
			nextSequenceState = NULL;
			shouldPlayerStandStill = false;
			lastAnimDict = NULL;
			lastAnim = NULL;
			camera = NULL;
			flashUnit = NULL;
		}
		break;
	}

	//Play PTFXs
	PlayPTFXAndSound();

	//Disable ped gestures and block non-player peds from reacting to temporary events
	SetPedMovementAndReactions();
	return;
}

void cCameraSequence::SetState(int state)
{
	if (state == EXITING && sequenceState != EXITING && sequenceState != FLUSH_ASSETS && sequenceState != FINISHED)
		sequenceState = EXITING;
	else if (state != EXITING)
		sequenceState = state;

	return;
}

void cCameraSequence::UpdateControls()
{
	if (sequenceState == WAITING_FOR_ANIMATION_TO_END || sequenceState == TAKE_PHOTO || 
		sequenceState == EXITING || sequenceState == FLUSH_ASSETS || sequenceState == FINISHED)
		return;

	AddScaleformInstructionalButton(control, input, "Take a picture (hold to stop)", true);
	RunScaleformInstructionalButtons();

	if (IS_DISABLED_CONTROL_JUST_PRESSED(control, input))
	{
		controlTimer.Set(0);
		return;
	}

	if (controlTimer.Get() < holdTime)
	{
		if (IS_DISABLED_CONTROL_JUST_RELEASED(control, input))
			SetState(TAKE_PHOTO);
	}
	else if (IS_DISABLED_CONTROL_PRESSED(control, input))
		shouldStopSequence = true;

	return;
}

void cCameraSequence::ForceStop()
{
	if (sequenceState == FLUSH_ASSETS || sequenceState == FINISHED)
		return;

	RESET_PED_WEAPON_MOVEMENT_CLIPSET(playerPed);
	shouldStopSequence = false;
	sequenceState = FLUSH_ASSETS;
	PlaySequence();
	DeleteObject(&camera);
	DeleteObject(&flashUnit);
	return;
}

void cCameraSequence::Update()
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
#pragma endregion