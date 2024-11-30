//ScriptHook
#include <natives.h>
// #include <types.h> //Already included in globals.h
//Custom
#include "actions.h"
#include "functions.h"
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
cMobileTextSequence mobileTextSequence;
cShineTorchSequence shineTorchSequence;
cLiftCurlBarSequence liftCurlBarSequence;
cBinocularsSequence binocularsSequence;
cHoldBumSignSequence holdBumSignSequence;
cFishingSequence fishingSequence;

namespace
{
constexpr int maxSequences = 16;
constexpr cSequence* sequences[maxSequences] = {
	&smokingSequence, &drinkingSequence, &leafBlowerSequence, &jogSequence, &clipboardSequence,
	&guitarSequence, &bongosSequence, &mopSequence, &mopWithBucketSequence, &cameraSequence,
	&mobileTextSequence, &shineTorchSequence, &liftCurlBarSequence, &binocularsSequence,
	&holdBumSignSequence, &fishingSequence
};

constexpr char* CORE_PTFX_ASSET = "core";

bool NoSequenceIsActive()
{
	LOOP(i, maxSequences)
	{
		if (sequences[i]->IsActive())
			return false;
	}

	return true;
}

void StopActiveSequence()
{
	LOOP(i, maxSequences)
	{
		if (sequences[i]->IsActive())
			sequences[i]->Stop();
	}

	return;
}
}

void UpdateSequences() { LOOP(i, maxSequences) { sequences[i]->Update(); } return; }

#pragma region Sequence
void cSequence::Start()
{
	if (!NoSequenceIsActive())
	{
		StopActiveSequence();
		return;
	}

	if (AdditionalChecks(GetPlayerPed()))
		sequenceState = SEQUENCE_STREAM_ASSETS_IN;

	return;
}

void cSequence::PlayAnimAndWait(char *animDict, char *anim, int flag, int nextState, float startPhase, float blendInSpeed, float blendOutSpeed, bool standStill, int duration)
{
	PlayAnimTask(GetPlayerPed(), animDict, anim, flag, startPhase, blendInSpeed, blendOutSpeed, duration);
	sequenceState = SEQUENCE_WAITING_FOR_ANIMATION_TO_END;
	nextSequenceState = nextState;
	shouldPlayerStandStill = standStill;
	lastAnimDict = animDict;
	lastAnim = anim;
	return;
}

void cSequence::PlayScriptedAnimAndWait(const int nextState, char* animDict, char* anim, const float phase, const float rate, const float weight, const int type, const int filter, const float blendInDelta, const float blendOutDelta, const int timeToPlay, const int flags, const int ikFlags, const bool standStill)
{
	PlayScriptedAnim(GetPlayerPed(), animDict, anim, phase, rate, weight, type, filter, blendInDelta, blendOutDelta, timeToPlay, flags, ikFlags);
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
		DisablePedConfigFlag(GetPlayerPed(), PCF_PhoneDisableTextingAnimations);
		DisablePedConfigFlag(GetPlayerPed(), PCF_PhoneDisableTalkingAnimations);
		DisablePedConfigFlag(GetPlayerPed(), PCF_PhoneDisableCameraAnimations);
		SET_PED_CAN_PLAY_GESTURE_ANIMS(GetPlayerPed(), true);
		SET_PED_CAN_PLAY_AMBIENT_ANIMS(GetPlayerPed(), true);
		SET_PED_CAN_PLAY_AMBIENT_BASE_ANIMS(GetPlayerPed(), true);
		SET_PED_CAN_PLAY_AMBIENT_IDLES(GetPlayerPed(), false, false);
		SET_PED_STEALTH_MOVEMENT(GetPlayerPed(), true, NULL);
		SET_PED_USING_ACTION_MODE(GetPlayerPed(), true, 0, NULL);
	}
	else
	{
		if (disableFirstPersonView)
			DISABLE_ON_FOOT_FIRST_PERSON_VIEW_THIS_UPDATE();

		EnablePedResetFlag(GetPlayerPed(), PRF_DisableSecondaryAnimationTasks);
		EnablePedConfigFlag(GetPlayerPed(), PCF_PhoneDisableTextingAnimations);
		EnablePedConfigFlag(GetPlayerPed(), PCF_PhoneDisableTalkingAnimations);
		EnablePedConfigFlag(GetPlayerPed(), PCF_PhoneDisableCameraAnimations);
		SET_PED_CAN_PLAY_GESTURE_ANIMS(GetPlayerPed(), false);
		SET_PED_CAN_PLAY_AMBIENT_ANIMS(GetPlayerPed(), false);
		SET_PED_CAN_PLAY_AMBIENT_BASE_ANIMS(GetPlayerPed(), false);
		SET_PED_CAN_PLAY_AMBIENT_IDLES(GetPlayerPed(), true, true);
		SET_PED_STEALTH_MOVEMENT(GetPlayerPed(), false, NULL);
		SET_PED_USING_ACTION_MODE(GetPlayerPed(), false, -1, NULL);
	}
	return;
}
#pragma endregion

//////////////////////////////////SMOKING//////////////////////////////
#pragma region Smoking
static constexpr int cigaretteHash = 0xC94C4B4C;		//Prop_CS_Ciggy_01					Prop_AMB_Ciggy_01
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
	StopAnimTask(GetPlayerPed(), smokeEnterAnimDict, smokeEnterAnim);
	StopAnimTask(GetPlayerPed(), smokeBaseAnimDict, smokeBaseAnim);
	StopAnimTask(GetPlayerPed(), smokeExitAnimDict, smokeExitAnim);
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
	const float enterAnimTime = GET_ENTITY_ANIM_CURRENT_TIME(GetPlayerPed(), smokeEnterAnimDict, smokeEnterAnim);
	if (enterAnimTime >= 0.4826087f && cigarettePTFXHandle == NULL)
		cigarettePTFXHandle = StartParticleFxLoopedOnPedBone(CORE_PTFX_ASSET, ANM_CIG_SMOKE, GetPlayerPed(), BONETAG_PH_R_HAND, 1.0f, -0.08f);

	const float baseAnimTime = GET_ENTITY_ANIM_CURRENT_TIME(GetPlayerPed(), smokeBaseAnimDict, smokeBaseAnim);
	if (baseAnimTime < 0.7965517f)
		hasExhaledNose = NULL;

	if (baseAnimTime >= 0.7965517f && hasExhaledNose == NULL)
		hasExhaledNose = StartParticleFxNonLoopedOnPedBone(CORE_PTFX_ASSET, ANM_CIG_EXHALE_NSE, GetPlayerPed(), BONETAG_HEAD, 1.0f, 0.02f, 0.16f);

	return;
}

void cSmokingSequence::PlaySequence()
{
	const int rightHandID = GET_PED_BONE_INDEX(GetPlayerPed(), BONETAG_PH_R_HAND);

	SetPlayerControls(); //Player control should be disabled here and not during the sequence

	switch (sequenceState)
	{
	case INITIALIZED:
		item = CreateObject(cigaretteHash);
		SET_ENTITY_AS_MISSION_ENTITY(item, false, true);
		ATTACH_ENTITY_TO_ENTITY(item, GetPlayerPed(), rightHandID, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, true, true, false, false, 2, true);
		PlayScriptedAnim(GetPlayerPed(), smokeEnterAnimDict, smokeEnterAnim, 0.0f, 1.0f, 1.0f, APT_SINGLE_ANIM, BONEMASK_HEAD_NECK_AND_R_ARM, 0.5f, NORMAL_BLEND_DURATION, -1, upperSecondaryAF | AF_HOLD_LAST_FRAME);
		sequenceState = ENTER_SMOKE;
		break;
	case WAITING_FOR_ANIMATION_TO_END:
		if (!IS_ENTITY_PLAYING_ANIM(GetPlayerPed(), lastAnimDict, lastAnim, 3) ||
			HAS_ENTITY_ANIM_FINISHED(GetPlayerPed(), lastAnimDict, lastAnim, 3))
			sequenceState = nextSequenceState;
		break;
	case ENTER_SMOKE:
		if (GET_ENTITY_ANIM_CURRENT_TIME(GetPlayerPed(), smokeEnterAnimDict, smokeEnterAnim) == 1.0f && !releasedArmAnim)
		{
			releasedArmAnim = true;
			PlayScriptedAnim(GetPlayerPed(), smokeEnterAnimDict, smokeEnterAnim, 1.0f, 1.0f, 1.0f, APT_SINGLE_ANIM, FILTER_GRIP_R_ONLY, NORMAL_BLEND_DURATION, NORMAL_BLEND_DURATION, -1, upperSecondaryAF | AF_HOLD_LAST_FRAME);
		}
		else if (!IS_ENTITY_PLAYING_ANIM(GetPlayerPed(), smokeEnterAnimDict, smokeEnterAnim, 3))
			PlayScriptedAnim(GetPlayerPed(), smokeEnterAnimDict, smokeEnterAnim, 0.0f, 1.0f, 1.0f, APT_SINGLE_ANIM, BONEMASK_HEAD_NECK_AND_R_ARM, 0.5f, NORMAL_BLEND_DURATION, -1, upperSecondaryAF | AF_HOLD_LAST_FRAME);
		break;
	case SMOKE:
		if (GET_ENTITY_ANIM_CURRENT_TIME(GetPlayerPed(), smokeBaseAnimDict, smokeBaseAnim) == 1.0f && !releasedArmAnim)
		{
			releasedArmAnim = true;
			PlayScriptedAnim(GetPlayerPed(), smokeBaseAnimDict, smokeBaseAnim, 1.0f, 1.0f, 1.0f, APT_SINGLE_ANIM, FILTER_GRIP_R_ONLY, NORMAL_BLEND_DURATION, NORMAL_BLEND_DURATION, -1, upperSecondaryAF | AF_HOLD_LAST_FRAME);
		}
		else if (!IS_ENTITY_PLAYING_ANIM(GetPlayerPed(), smokeBaseAnimDict, smokeBaseAnim, 3))
			PlayScriptedAnim(GetPlayerPed(), smokeBaseAnimDict, smokeBaseAnim, 0.0f, 1.0f, 1.0f, APT_SINGLE_ANIM, BONEMASK_HEAD_NECK_AND_R_ARM, 0.5f, NORMAL_BLEND_DURATION, -1, upperSecondaryAF | AF_HOLD_LAST_FRAME);
		break;
	case EXITING:
		PlayScriptedAnimAndWait(FLUSH_ASSETS, smokeExitAnimDict, smokeExitAnim, 0.0f, 1.0f, 1.0f, APT_SINGLE_ANIM, BONEMASK_HEAD_NECK_AND_R_ARM, NORMAL_BLEND_DURATION, NORMAL_BLEND_DURATION, -1, upperSecondaryAF);
		break;
	case FLUSH_ASSETS:
		DeleteEntity(&item);
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
			releasedArmAnim = false;
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
	if (HAS_ENTITY_ANIM_FINISHED(GetPlayerPed(), smokeEnterAnimDict, smokeEnterAnim, 3))
	{
		*animDict = smokeEnterAnimDict;
		*anim = smokeEnterAnim;
	}
	else if (HAS_ENTITY_ANIM_FINISHED(GetPlayerPed(), smokeBaseAnimDict, smokeBaseAnim, 3))
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

	StopAnimTask(GetPlayerPed(), animDict, anim);

	if (state == EXITING && sequenceState != WAITING_FOR_ANIMATION_TO_END && sequenceState != EXITING && sequenceState != FLUSH_ASSETS && sequenceState != FINISHED)
		sequenceState = EXITING;
	else if (state != EXITING && state != sequenceState)
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

	if (instructionalButtonsText != NULL)
	{
		AddScaleformInstructionalButton(control, input, instructionalButtonsText, true);
		RunScaleformInstructionalButtons();
	}

	if (IS_DISABLED_CONTROL_JUST_PRESSED(control, input))
	{
		controlTimer.Reset();
		return;
	}

	if (controlTimer.Get() < holdTime)
	{
		if (IS_DISABLED_CONTROL_JUST_RELEASED(control, input))
		{
			SetState(SMOKE);
			releasedArmAnim = false;
		}
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
	DeleteEntity(&item);
	shouldStopSequence = false;
	sequenceState = FLUSH_ASSETS;
	PlaySequence();
	return;
}

void cSmokingSequence::Update()
{
	if (sequenceState != FINISHED)
	{
		if (!AdditionalChecks(GetPlayerPed()))
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
			stopTimer.Reset();

		UpdateControls();
		return;
	}

	shouldStopSequence = false; //Reset var
	DeleteEntity(&item); //Force delete old item
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
	StopAnimTask(GetPlayerPed(), drinkingAnimDict, drinkingEnterAnim);
	StopAnimTask(GetPlayerPed(), drinkingAnimDict, drinkingBaseAnim);
	StopAnimTask(GetPlayerPed(), drinkingAnimDict, drinkingExitAnim);
	REMOVE_ANIM_DICT(drinkingAnimDict);
	return;
}

void cDrinkingSequence::PlaySequence()
{
	const int leftHandID = GET_PED_BONE_INDEX(GetPlayerPed(), BONETAG_PH_L_HAND);

	SetPlayerControls(); //Player control should be disabled here and not during the sequence

	switch (sequenceState)
	{
	case INITIALIZED:
		item = CreateObject(beerHash);
		SET_ENTITY_AS_MISSION_ENTITY(item, false, true);
		ATTACH_ENTITY_TO_ENTITY(item, GetPlayerPed(), leftHandID, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, true, true, false, false, 2, true);
		PlayAnimAndWait(drinkingAnimDict, drinkingEnterAnim, upperSecondaryAF, DRINK, 0.35f, SLOW_BLEND_IN, SLOWEST_BLEND_OUT);
		break;
	case WAITING_FOR_ANIMATION_TO_END:
		if (!IS_ENTITY_PLAYING_ANIM(GetPlayerPed(), lastAnimDict, lastAnim, 3) ||
			HAS_ENTITY_ANIM_FINISHED(GetPlayerPed(), lastAnimDict, lastAnim, 3))
			sequenceState = nextSequenceState;

		SetAnimSpeed(GetPlayerPed(), drinkingAnimDict, drinkingEnterAnim, 0.7f);
		break;
	case ENTER_DRINK:
		PlayAnimAndWait(drinkingAnimDict, drinkingEnterAnim, upperSecondaryAF, DRINK, 0.35f, SLOW_BLEND_IN, SLOWEST_BLEND_OUT);
		break;
	case DRINK:
		PlayAnimAndWait(drinkingAnimDict, drinkingBaseAnim, upperSecondaryAF, HOLD, 0.0f, FAST_BLEND_IN, SLOWEST_BLEND_OUT);
		break;
	case HOLD:
		if (!IS_ENTITY_PLAYING_ANIM(GetPlayerPed(), drinkingAnimDict, drinkingExitAnim, 3))
			PlayAnimTask(GetPlayerPed(), drinkingAnimDict, drinkingExitAnim, upperSecondaryAF | AF_HOLD_LAST_FRAME);
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
	if (HAS_ENTITY_ANIM_FINISHED(GetPlayerPed(), drinkingAnimDict, drinkingExitAnim, 3))
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

	if (state == EXITING && sequenceState != WAITING_FOR_ANIMATION_TO_END && sequenceState != EXITING && sequenceState != FLUSH_ASSETS && sequenceState != FINISHED)
		sequenceState = EXITING;
	else if (state != EXITING && state != sequenceState)
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

	if (instructionalButtonsText != NULL)
	{
		AddScaleformInstructionalButton(control, input, instructionalButtonsText, true);
		RunScaleformInstructionalButtons();
	}

	if (IS_DISABLED_CONTROL_JUST_PRESSED(control, input))
	{
		controlTimer.Reset();
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
	DeleteEntity(&item);
	return;
}

void cDrinkingSequence::Update()
{
	if (sequenceState != FINISHED)
	{
		if (!AdditionalChecks(GetPlayerPed()))
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
			stopTimer.Reset();

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
		leafBlowerPTFXHandle = StartParticleFxLoopedOnPedBone(CORE_PTFX_ASSET, ANM_LEAF_BLOWER, GetPlayerPed(), BONETAG_PH_R_HAND, 1.0f, 0.9f, 0.0f, -0.25f);
		soundID = GET_SOUND_ID();
		PLAY_SOUND_FROM_ENTITY(soundID, leafBlowerSound, GetPlayerPed(), NULL, false, 0);
	}
	return;
}

void cLeafBlowerSequence::PlaySequence()
{
	const int rightHandID = GET_PED_BONE_INDEX(GetPlayerPed(), BONETAG_PH_R_HAND);

	SetPlayerControls(); //Player control should be disabled here and not during the sequence

	switch (sequenceState)
	{
	case INITIALIZED:
		item = CreateObject(leafBlowerHash);
		SET_ENTITY_AS_MISSION_ENTITY(item, false, true);
		ATTACH_ENTITY_TO_ENTITY(item, GetPlayerPed(), rightHandID, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, true, true, false, false, 2, true);
		SET_PED_WEAPON_MOVEMENT_CLIPSET(GetPlayerPed(), leafBlowerClipSet);
		TASK_LOOK_AT_ENTITY(GetPlayerPed(), item, -1, SLF_NARROW_YAW_LIMIT | SLF_NARROW_PITCH_LIMIT | SLF_SLOW_TURN_RATE, 2);
		sequenceState = LOOP;
		break;
	case WAITING_FOR_ANIMATION_TO_END:
		if (!IS_ENTITY_PLAYING_ANIM(GetPlayerPed(), lastAnimDict, lastAnim, 3) ||
			HAS_ENTITY_ANIM_FINISHED(GetPlayerPed(), lastAnimDict, lastAnim, 3))
			sequenceState = nextSequenceState;
		break;
	case LOOP:
		break;
	case EXITING:
		RESET_PED_WEAPON_MOVEMENT_CLIPSET(GetPlayerPed());
		TASK_CLEAR_LOOK_AT(GetPlayerPed());
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
	if (state == EXITING && sequenceState != WAITING_FOR_ANIMATION_TO_END && sequenceState != EXITING && sequenceState != FLUSH_ASSETS && sequenceState != FINISHED)
		sequenceState = EXITING;
	else if (state != EXITING && state != sequenceState)
		sequenceState = state;

	return;
}

void cLeafBlowerSequence::UpdateControls()
{
	if (sequenceState != LOOP)
		return;

	if (instructionalButtonsText != NULL)
	{
		AddScaleformInstructionalButton(control, input, instructionalButtonsText, true);
		RunScaleformInstructionalButtons();
	}

	if (IS_DISABLED_CONTROL_JUST_PRESSED(control, input))
	{
		controlTimer.Reset();
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

	RESET_PED_WEAPON_MOVEMENT_CLIPSET(GetPlayerPed());
	TASK_CLEAR_LOOK_AT(GetPlayerPed());
	StopAllPTFXAndSounds();
	shouldStopSequence = false;
	sequenceState = FLUSH_ASSETS;
	PlaySequence();
	DeleteEntity(&item);
	return;
}

void cLeafBlowerSequence::Update()
{
	if (sequenceState != FINISHED)
	{
		if (!AdditionalChecks(GetPlayerPed()))
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
			stopTimer.Reset();

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
		SET_PED_ALTERNATE_MOVEMENT_ANIM(GetPlayerPed(), AAT_IDLE, joggerAnimDict, joggerIdleAnim, 8.0f, true);
		SET_PED_ALTERNATE_MOVEMENT_ANIM(GetPlayerPed(), AAT_WALK, joggerAnimDict, joggerRunAnim, 8.0f, true);
		sequenceState = LOOP;
		break;
	case WAITING_FOR_ANIMATION_TO_END:
		if (!IS_ENTITY_PLAYING_ANIM(GetPlayerPed(), lastAnimDict, lastAnim, 3) ||
			HAS_ENTITY_ANIM_FINISHED(GetPlayerPed(), lastAnimDict, lastAnim, 3))
			sequenceState = nextSequenceState;
		break;
	case LOOP:
		break;
	case EXITING:
		CLEAR_PED_ALTERNATE_MOVEMENT_ANIM(GetPlayerPed(), AAT_IDLE, 8.0f);
		CLEAR_PED_ALTERNATE_MOVEMENT_ANIM(GetPlayerPed(), AAT_WALK, 8.0f);
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
	if (state == EXITING && sequenceState != WAITING_FOR_ANIMATION_TO_END && sequenceState != EXITING && sequenceState != FLUSH_ASSETS && sequenceState != FINISHED)
		sequenceState = EXITING;
	else if (state != EXITING && state != sequenceState)
		sequenceState = state;

	return;
}

void cJogSequence::UpdateControls()
{
	if (sequenceState != LOOP)
		return;

	if (instructionalButtonsText != NULL)
	{
		AddScaleformInstructionalButton(control, input, instructionalButtonsText, true);
		RunScaleformInstructionalButtons();
	}

	if (IS_DISABLED_CONTROL_JUST_PRESSED(control, input))
	{
		controlTimer.Reset();
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

	CLEAR_PED_ALTERNATE_MOVEMENT_ANIM(GetPlayerPed(), AAT_IDLE, 8.0f);
	CLEAR_PED_ALTERNATE_MOVEMENT_ANIM(GetPlayerPed(), AAT_WALK, 8.0f);
	shouldStopSequence = false;
	sequenceState = FLUSH_ASSETS;
	PlaySequence();
	return;
}

void cJogSequence::Update()
{
	if (sequenceState != FINISHED)
	{
		if (!AdditionalChecks(GetPlayerPed()))
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
			stopTimer.Reset();

		UpdateControls();
		return;
	}

	shouldStopSequence = false; //Reset var
	return;
}
#pragma endregion

//////////////////////////////////CLIPBOARD//////////////////////////
#pragma region Clipboard
static constexpr int clipboardHash = 0xC638E52B;		//P_AMB_Clipboard_01
static constexpr char* clipboardClipSet = "move_m@clipboard";

void cClipboardSequence::PlaySequence()
{
	const int leftHandID = GET_PED_BONE_INDEX(GetPlayerPed(), BONETAG_PH_L_HAND);

	SetPlayerControls(); //Player control should be disabled here and not during the sequence

	switch (sequenceState)
	{
	case INITIALIZED:
		item = CreateObject(clipboardHash);
		SET_ENTITY_AS_MISSION_ENTITY(item, false, true);
		ATTACH_ENTITY_TO_ENTITY(item, GetPlayerPed(), leftHandID, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, true, true, false, false, 2, true);
		SET_PED_WEAPON_MOVEMENT_CLIPSET(GetPlayerPed(), clipboardClipSet);
		TASK_LOOK_AT_ENTITY(GetPlayerPed(), item, -1, SLF_NARROW_YAW_LIMIT | SLF_NARROW_PITCH_LIMIT | SLF_SLOW_TURN_RATE, 2);
		sequenceState = LOOP;
		break;
	case WAITING_FOR_ANIMATION_TO_END:
		if (!IS_ENTITY_PLAYING_ANIM(GetPlayerPed(), lastAnimDict, lastAnim, 3) ||
			HAS_ENTITY_ANIM_FINISHED(GetPlayerPed(), lastAnimDict, lastAnim, 3))
			sequenceState = nextSequenceState;
		break;
	case LOOP:
		break;
	case EXITING:
		RESET_PED_WEAPON_MOVEMENT_CLIPSET(GetPlayerPed());
		TASK_CLEAR_LOOK_AT(GetPlayerPed());
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
	if (state == EXITING && sequenceState != WAITING_FOR_ANIMATION_TO_END && sequenceState != EXITING && sequenceState != FLUSH_ASSETS && sequenceState != FINISHED)
		sequenceState = EXITING;
	else if (state != EXITING && state != sequenceState)
		sequenceState = state;

	return;
}

void cClipboardSequence::UpdateControls()
{
	if (sequenceState != LOOP)
		return;

	if (instructionalButtonsText != NULL)
	{
		AddScaleformInstructionalButton(control, input, instructionalButtonsText, true);
		RunScaleformInstructionalButtons();
	}

	if (IS_DISABLED_CONTROL_JUST_PRESSED(control, input))
	{
		controlTimer.Reset();
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

	RESET_PED_WEAPON_MOVEMENT_CLIPSET(GetPlayerPed());
	TASK_CLEAR_LOOK_AT(GetPlayerPed());
	shouldStopSequence = false;
	sequenceState = FLUSH_ASSETS;
	PlaySequence();
	DeleteEntity(&item);
	return;
}

void cClipboardSequence::Update()
{
	if (sequenceState != FINISHED)
	{
		if (!AdditionalChecks(GetPlayerPed()))
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
			stopTimer.Reset();

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
	StopAnimTask(GetPlayerPed(), guitarAnimDict, guitarBaseAnim);
	REMOVE_ANIM_DICT(guitarAnimDict);
	return;
}

void cGuitarSequence::PlaySequence()
{
	const int leftHandID = GET_PED_BONE_INDEX(GetPlayerPed(), BONETAG_PH_L_HAND);

	SetPlayerControls(); //Player control should be disabled here and not during the sequence

	switch (sequenceState)
	{
	case INITIALIZED:
		StopAudioStream();
		item = CreateObject(guitarHash);
		SET_ENTITY_AS_MISSION_ENTITY(item, false, true);
		ATTACH_ENTITY_TO_ENTITY(item, GetPlayerPed(), leftHandID, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, true, true, false, false, 2, true);
		PlayAnimTask(GetPlayerPed(), guitarAnimDict, guitarBaseAnim, upperSecondaryAF | AF_LOOPING);
		sequenceState = LOOP;
		break;
	case WAITING_FOR_ANIMATION_TO_END:
		if (!IS_ENTITY_PLAYING_ANIM(GetPlayerPed(), lastAnimDict, lastAnim, 3) ||
			HAS_ENTITY_ANIM_FINISHED(GetPlayerPed(), lastAnimDict, lastAnim, 3))
			sequenceState = nextSequenceState;
		break;
	case LOOP:
		if (!IS_STREAM_PLAYING())
		{
			if (LOAD_STREAM(guitarStream, NULL))
				PLAY_STREAM_FROM_PED(GetPlayerPed());
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
	StopAnimTask(GetPlayerPed(), guitarAnimDict, guitarBaseAnim);

	if (state == EXITING && sequenceState != WAITING_FOR_ANIMATION_TO_END && sequenceState != EXITING && sequenceState != FLUSH_ASSETS && sequenceState != FINISHED)
		sequenceState = EXITING;
	else if (state != EXITING && state != sequenceState)
		sequenceState = state;

	return;
}

void cGuitarSequence::UpdateControls()
{
	if (sequenceState != LOOP)
		return;

	if (instructionalButtonsText != NULL)
	{
		AddScaleformInstructionalButton(control, input, instructionalButtonsText, true);
		RunScaleformInstructionalButtons();
	}

	if (IS_DISABLED_CONTROL_JUST_PRESSED(control, input))
	{
		controlTimer.Reset();
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
	DeleteEntity(&item);
	return;
}

void cGuitarSequence::Update()
{
	if (sequenceState != FINISHED)
	{
		if (!AdditionalChecks(GetPlayerPed()))
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
			stopTimer.Reset();

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
	StopAnimTask(GetPlayerPed(), bongosAnimDict, bongosBaseAnim);
	REMOVE_ANIM_DICT(bongosAnimDict);
	return;
}

void cBongosSequence::PlaySequence()
{
	const int leftHandID = GET_PED_BONE_INDEX(GetPlayerPed(), BONETAG_PH_L_HAND);

	SetPlayerControls(); //Player control should be disabled here and not during the sequence

	switch (sequenceState)
	{
	case INITIALIZED:
		StopAudioStream();
		item = CreateObject(bongosHash);
		SET_ENTITY_AS_MISSION_ENTITY(item, false, true);
		ATTACH_ENTITY_TO_ENTITY(item, GetPlayerPed(), leftHandID, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, true, true, false, false, 2, true);
		PlayAnimTask(GetPlayerPed(), bongosAnimDict, bongosBaseAnim, upperSecondaryAF | AF_LOOPING);
		sequenceState = LOOP;
		break;
	case WAITING_FOR_ANIMATION_TO_END:
		if (!IS_ENTITY_PLAYING_ANIM(GetPlayerPed(), lastAnimDict, lastAnim, 3) ||
			HAS_ENTITY_ANIM_FINISHED(GetPlayerPed(), lastAnimDict, lastAnim, 3))
			sequenceState = nextSequenceState;
		break;
	case LOOP:
		if (!IS_STREAM_PLAYING())
		{
			if (LOAD_STREAM(bongosStream, NULL))
				PLAY_STREAM_FROM_PED(GetPlayerPed());
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
	StopAnimTask(GetPlayerPed(), bongosAnimDict, bongosBaseAnim);

	if (state == EXITING && sequenceState != WAITING_FOR_ANIMATION_TO_END && sequenceState != EXITING && sequenceState != FLUSH_ASSETS && sequenceState != FINISHED)
		sequenceState = EXITING;
	else if (state != EXITING && state != sequenceState)
		sequenceState = state;

	return;
}

void cBongosSequence::UpdateControls()
{
	if (sequenceState != LOOP)
		return;

	if (instructionalButtonsText != NULL)
	{
		AddScaleformInstructionalButton(control, input, instructionalButtonsText, true);
		RunScaleformInstructionalButtons();
	}

	if (IS_DISABLED_CONTROL_JUST_PRESSED(control, input))
	{
		controlTimer.Reset();
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
	DeleteEntity(&item);
	return;
}

void cBongosSequence::Update()
{
	if (sequenceState != FINISHED)
	{
		if (!AdditionalChecks(GetPlayerPed()))
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
			stopTimer.Reset();

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
static constexpr char* fibJanitorAnimDict = "missfbi_s4mop";
static constexpr char* mopIdleAnim = "mop_idle";

void cMopSequence::PlaySequence()
{
	const int leftHandID = GET_PED_BONE_INDEX(GetPlayerPed(), BONETAG_PH_L_HAND);

	SetPlayerControls(); //Player control should be disabled here and not during the sequence

	switch (sequenceState)
	{
	case INITIALIZED:
		item = CreateObject(mopHash);
		SET_ENTITY_AS_MISSION_ENTITY(item, false, true);
		ATTACH_ENTITY_TO_ENTITY(item, GetPlayerPed(), leftHandID, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, true, true, false, false, 2, true);
		PLAY_ENTITY_ANIM(item, mopIdleAnim, fibJanitorAnimDict, INSTANT_BLEND_IN, true, false, false, 0.0f, NULL);

		SET_PED_WEAPON_MOVEMENT_CLIPSET(GetPlayerPed(), mopClipSet);
		sequenceState = LOOP;
		break;
	case WAITING_FOR_ANIMATION_TO_END:
		if (!IS_ENTITY_PLAYING_ANIM(GetPlayerPed(), lastAnimDict, lastAnim, 3) ||
			HAS_ENTITY_ANIM_FINISHED(GetPlayerPed(), lastAnimDict, lastAnim, 3))
			sequenceState = nextSequenceState;
		break;
	case LOOP:
		break;
	case EXITING:
		RESET_PED_WEAPON_MOVEMENT_CLIPSET(GetPlayerPed());
		sequenceState = FLUSH_ASSETS;
		break;
	case FLUSH_ASSETS:
		DETACH_ENTITY(item, false, false);
		SET_ENTITY_AS_NO_LONGER_NEEDED(&item);
		SET_MODEL_AS_NO_LONGER_NEEDED(mopHash);
		REMOVE_ANIM_DICT(fibJanitorAnimDict);
		REMOVE_CLIP_SET(mopClipSet);
		shouldStopSequence = false;
		sequenceState = FINISHED;
		break;
	case STREAM_ASSETS_IN:
		if (RequestModel(mopHash) && RequestClipSet(mopClipSet) && RequestAnimDict(fibJanitorAnimDict))
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
	if (state == EXITING && sequenceState != WAITING_FOR_ANIMATION_TO_END && sequenceState != EXITING && sequenceState != FLUSH_ASSETS && sequenceState != FINISHED)
		sequenceState = EXITING;
	else if (state != EXITING && state != sequenceState)
		sequenceState = state;

	return;
}

void cMopSequence::UpdateControls()
{
	if (sequenceState != LOOP)
		return;

	if (instructionalButtonsText != NULL)
	{
		AddScaleformInstructionalButton(control, input, instructionalButtonsText, true);
		RunScaleformInstructionalButtons();
	}

	if (IS_DISABLED_CONTROL_JUST_PRESSED(control, input))
	{
		controlTimer.Reset();
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

	RESET_PED_WEAPON_MOVEMENT_CLIPSET(GetPlayerPed());
	shouldStopSequence = false;
	sequenceState = FLUSH_ASSETS;
	PlaySequence();
	DeleteEntity(&item);
	return;
}

void cMopSequence::Update()
{
	if (sequenceState != FINISHED)
	{
		if (!AdditionalChecks(GetPlayerPed()))
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
			stopTimer.Reset();

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
static constexpr char* bucketIdleAnim = "bucket_idle";

void cMopWithBucketSequence::PlaySequence()
{
	const int rightHandID = GET_PED_BONE_INDEX(GetPlayerPed(), BONETAG_PH_R_HAND);
	const int leftHandID = GET_PED_BONE_INDEX(GetPlayerPed(), BONETAG_PH_L_HAND);

	SetPlayerControls(); //Player control should be disabled here and not during the sequence

	switch (sequenceState)
	{
	case INITIALIZED:
		bucket = CreateObject(bucketHash);
		SET_ENTITY_AS_MISSION_ENTITY(bucket, false, true);
		ATTACH_ENTITY_TO_ENTITY(bucket, GetPlayerPed(), rightHandID, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, true, true, false, false, 2, true);
		PLAY_ENTITY_ANIM(bucket, bucketIdleAnim, fibJanitorAnimDict, INSTANT_BLEND_IN, true, false, false, 0.0f, NULL);

		mop = CreateObject(mopHash);
		SET_ENTITY_AS_MISSION_ENTITY(mop, false, true);
		ATTACH_ENTITY_TO_ENTITY(mop, GetPlayerPed(), leftHandID, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, true, true, false, false, 2, true);
		PLAY_ENTITY_ANIM(mop, mopIdleAnim, fibJanitorAnimDict, INSTANT_BLEND_IN, true, false, false, 0.0f, NULL);

		SET_PED_WEAPON_MOVEMENT_CLIPSET(GetPlayerPed(), bucketClipSet);
		sequenceState = LOOP;
		break;
	case WAITING_FOR_ANIMATION_TO_END:
		if (!IS_ENTITY_PLAYING_ANIM(GetPlayerPed(), lastAnimDict, lastAnim, 3) ||
			HAS_ENTITY_ANIM_FINISHED(GetPlayerPed(), lastAnimDict, lastAnim, 3))
			sequenceState = nextSequenceState;
		break;
	case LOOP:
		break;
	case EXITING:
		RESET_PED_WEAPON_MOVEMENT_CLIPSET(GetPlayerPed());
		sequenceState = FLUSH_ASSETS;
		break;
	case FLUSH_ASSETS:
		DETACH_ENTITY(bucket, false, false);
		SET_ENTITY_AS_NO_LONGER_NEEDED(&bucket);
		SET_MODEL_AS_NO_LONGER_NEEDED(bucketHash);

		DETACH_ENTITY(mop, false, false);
		SET_ENTITY_AS_NO_LONGER_NEEDED(&mop);
		SET_MODEL_AS_NO_LONGER_NEEDED(mopHash);

		REMOVE_ANIM_DICT(fibJanitorAnimDict);
		REMOVE_CLIP_SET(bucketClipSet);
		shouldStopSequence = false;
		sequenceState = FINISHED;
		break;
	case STREAM_ASSETS_IN:
		if (RequestModel(bucketHash) && RequestModel(mopHash) && RequestAnimDict(fibJanitorAnimDict) && RequestClipSet(bucketClipSet))
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
	if (state == EXITING && sequenceState != WAITING_FOR_ANIMATION_TO_END && sequenceState != EXITING && sequenceState != FLUSH_ASSETS && sequenceState != FINISHED)
		sequenceState = EXITING;
	else if (state != EXITING && state != sequenceState)
		sequenceState = state;

	return;
}

void cMopWithBucketSequence::UpdateControls()
{
	if (sequenceState != LOOP)
		return;

	if (instructionalButtonsText != NULL)
	{
		AddScaleformInstructionalButton(control, input, instructionalButtonsText, true);
		RunScaleformInstructionalButtons();
	}

	if (IS_DISABLED_CONTROL_JUST_PRESSED(control, input))
	{
		controlTimer.Reset();
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

	RESET_PED_WEAPON_MOVEMENT_CLIPSET(GetPlayerPed());
	shouldStopSequence = false;
	sequenceState = FLUSH_ASSETS;
	PlaySequence();
	DeleteEntity(&bucket);
	DeleteEntity(&mop);
	return;
}

void cMopWithBucketSequence::Update()
{
	if (sequenceState != FINISHED)
	{
		if (!AdditionalChecks(GetPlayerPed()))
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
			stopTimer.Reset();

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

	if (playFlashSound && GET_ENTITY_ANIM_CURRENT_TIME(GetPlayerPed(), cameraClipSet, cameraActionAnim) > 0.5f)
	{	
		StartParticleFxNonLoopedOnEntity(CORE_PTFX_ASSET, ANM_PAPARAZZI_FLASH, flashUnit, 1.0f, -0.04f, -0.06f);
		PLAY_SOUND_FROM_ENTITY(-1, cameraSound, GetPlayerPed(), NULL, false, 0);
		playFlashSound = false;
	}
	return;
}

void cCameraSequence::PlaySequence()
{
	const int rightHandID = GET_PED_BONE_INDEX(GetPlayerPed(), BONETAG_PH_R_HAND);

	SetPlayerControls(); //Player control should be disabled here and not during the sequence

	switch (sequenceState)
	{
	case INITIALIZED:
		camera = CreateObject(cameraHash);
		SET_ENTITY_AS_MISSION_ENTITY(camera, false, true);
		ATTACH_ENTITY_TO_ENTITY(camera, GetPlayerPed(), rightHandID, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, true, true, false, false, 2, true);

		flashUnit = CreateObject(flashUnitHash);
		SET_ENTITY_AS_MISSION_ENTITY(flashUnit, false, true);
		ATTACH_ENTITY_TO_ENTITY(flashUnit, camera, NULL, 0.107f, 0.025f, 0.177f, 0.0f, 0.0f, 0.0f, true, true, false, false, 2, true);

		SET_PED_WEAPON_MOVEMENT_CLIPSET(GetPlayerPed(), cameraClipSet);
		sequenceState = LOOP;
		break;
	case WAITING_FOR_ANIMATION_TO_END:
		if (!IS_ENTITY_PLAYING_ANIM(GetPlayerPed(), lastAnimDict, lastAnim, 3) ||
			HAS_ENTITY_ANIM_FINISHED(GetPlayerPed(), lastAnimDict, lastAnim, 3))
			sequenceState = nextSequenceState;

		SetAnimSpeed(GetPlayerPed(), cameraClipSet, cameraActionAnim, 2.0f);
		break;
	case LOOP:
		break;
	case TAKE_PHOTO:
		if (!IS_ENTITY_PLAYING_ANIM(GetPlayerPed(), cameraClipSet, cameraActionAnim, 3))
		{
			PlayAnimAndWait(cameraClipSet, cameraActionAnim, upperSecondaryAF, LOOP);
			playFlashSound = true;
		}
		break;
	case EXITING:
		RESET_PED_WEAPON_MOVEMENT_CLIPSET(GetPlayerPed());
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
	if (state == EXITING && sequenceState != WAITING_FOR_ANIMATION_TO_END && sequenceState != EXITING && sequenceState != FLUSH_ASSETS && sequenceState != FINISHED)
		sequenceState = EXITING;
	else if (state != EXITING && state != sequenceState)
		sequenceState = state;

	return;
}

void cCameraSequence::UpdateControls()
{
	if (sequenceState != LOOP)
		return;

	if (instructionalButtonsText != NULL)
	{
		AddScaleformInstructionalButton(control, input, instructionalButtonsText, true);
		RunScaleformInstructionalButtons();
	}

	if (IS_DISABLED_CONTROL_JUST_PRESSED(control, input))
	{
		controlTimer.Reset();
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

	RESET_PED_WEAPON_MOVEMENT_CLIPSET(GetPlayerPed());
	shouldStopSequence = false;
	sequenceState = FLUSH_ASSETS;
	PlaySequence();
	DeleteEntity(&camera);
	DeleteEntity(&flashUnit);
	return;
}

void cCameraSequence::Update()
{
	if (sequenceState != FINISHED)
	{
		if (!AdditionalChecks(GetPlayerPed()))
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
			stopTimer.Reset();

		UpdateControls();
		return;
	}

	shouldStopSequence = false; //Reset var
	return;
}
#pragma endregion

//////////////////////////////////MOBILE TEXT//////////////////////////////
#pragma region Mobile Text
static constexpr int mobileHash = 0x3A1B896A;		//Prop_AMB_Phone
static constexpr char* mobileBaseAnimDict = "amb@world_human_stand_mobile@male@text@base";
static constexpr char* mobileBaseAnim = "base";

static constexpr char* mobileEnterAnimDict = "amb@world_human_stand_mobile@male@text@enter";
static constexpr char* mobileEnterAnim = "enter";

static constexpr char* mobileExitAnimDict = "amb@world_human_stand_mobile@male@text@exit";
static constexpr char* mobileExitAnim = "exit";

void cMobileTextSequence::StopAllAnims()
{
	StopAnimTask(GetPlayerPed(), mobileEnterAnimDict, mobileEnterAnim);
	StopAnimTask(GetPlayerPed(), mobileBaseAnimDict, mobileBaseAnim);
	StopAnimTask(GetPlayerPed(), mobileExitAnimDict, mobileExitAnim);
	REMOVE_ANIM_DICT(mobileEnterAnimDict);
	REMOVE_ANIM_DICT(mobileBaseAnimDict);
	REMOVE_ANIM_DICT(mobileExitAnimDict);
	return;
}

void cMobileTextSequence::PlaySequence()
{
	const int rightHandID = GET_PED_BONE_INDEX(GetPlayerPed(), BONETAG_PH_R_HAND);

	SetPlayerControls(); //Player control should be disabled here and not during the sequence

	switch (sequenceState)
	{
	case INITIALIZED:
		item = CreateObject(mobileHash);
		SET_ENTITY_AS_MISSION_ENTITY(item, false, true);
		ATTACH_ENTITY_TO_ENTITY(item, GetPlayerPed(), rightHandID, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, true, true, false, false, 2, true);
		SET_ENTITY_VISIBLE(item, false, false);

		PlayAnimAndWait(mobileEnterAnimDict, mobileEnterAnim, upperSecondaryAF | AF_HOLD_LAST_FRAME, LOOP, 0.0f, SLOW_BLEND_IN, SLOWEST_BLEND_OUT);
		break;
	case WAITING_FOR_ANIMATION_TO_END:
		if (!IS_ENTITY_PLAYING_ANIM(GetPlayerPed(), lastAnimDict, lastAnim, 3) ||
			HAS_ENTITY_ANIM_FINISHED(GetPlayerPed(), lastAnimDict, lastAnim, 3))
			sequenceState = nextSequenceState;

		if (GET_ENTITY_ANIM_CURRENT_TIME(GetPlayerPed(), mobileEnterAnimDict, mobileEnterAnim) > 0.225f)
			SET_ENTITY_VISIBLE(item, true, false);

		if (GET_ENTITY_ANIM_CURRENT_TIME(GetPlayerPed(), mobileExitAnimDict, mobileExitAnim) > 0.6f)
			DeleteEntity(&item);
		break;
	case LOOP:
		if (!IS_ENTITY_PLAYING_ANIM(GetPlayerPed(), mobileBaseAnimDict, mobileBaseAnim, 3))
			PlayAnimTask(GetPlayerPed(), mobileBaseAnimDict, mobileBaseAnim, upperSecondaryAF | AF_LOOPING, 0.0f, INSTANT_BLEND_IN, SLOWEST_BLEND_OUT);
		break;
	case EXITING:
		PlayAnimAndWait(mobileExitAnimDict, mobileExitAnim, upperSecondaryAF, FLUSH_ASSETS, 0.0f, INSTANT_BLEND_IN);
		break;
	case FLUSH_ASSETS:
		DeleteEntity(&item);
		SET_MODEL_AS_NO_LONGER_NEEDED(mobileHash);
		REMOVE_ANIM_DICT(mobileEnterAnimDict);
		REMOVE_ANIM_DICT(mobileBaseAnimDict);
		REMOVE_ANIM_DICT(mobileExitAnimDict);
		shouldStopSequence = false;
		sequenceState = FINISHED;
		break;
	case STREAM_ASSETS_IN:
		if (RequestModel(mobileHash) && RequestAnimDict(mobileEnterAnimDict) &&
			RequestAnimDict(mobileBaseAnimDict) && RequestAnimDict(mobileExitAnimDict))
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

void cMobileTextSequence::SetState(int state)
{
	if (IS_ENTITY_PLAYING_ANIM(GetPlayerPed(), mobileEnterAnimDict, mobileEnterAnim, 3) ||
		IS_ENTITY_PLAYING_ANIM(GetPlayerPed(), mobileExitAnimDict, mobileExitAnim, 3))
		return;

	if (state == EXITING && sequenceState != WAITING_FOR_ANIMATION_TO_END && sequenceState != EXITING && sequenceState != FLUSH_ASSETS && sequenceState != FINISHED)
		sequenceState = EXITING;
	else if (state != EXITING && state != sequenceState)
		sequenceState = state;

	return;
}

void cMobileTextSequence::UpdateControls()
{
	if (sequenceState != LOOP)
		return;

	if (instructionalButtonsText != NULL)
	{
		AddScaleformInstructionalButton(control, input, instructionalButtonsText, true);
		RunScaleformInstructionalButtons();
	}

	if (IS_DISABLED_CONTROL_JUST_PRESSED(control, input))
	{
		controlTimer.Reset();
		return;
	}

	if (controlTimer.Get() > holdTime && IS_DISABLED_CONTROL_PRESSED(control, input))
		shouldStopSequence = true;

	return;
}

void cMobileTextSequence::ForceStop()
{
	if (sequenceState == FLUSH_ASSETS || sequenceState == FINISHED)
		return;

	StopAllAnims();
	DeleteEntity(&item);
	shouldStopSequence = false;
	sequenceState = FLUSH_ASSETS;
	PlaySequence();
	return;
}

void cMobileTextSequence::Update()
{
	if (sequenceState != FINISHED)
	{
		if (!AdditionalChecks(GetPlayerPed()))
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
			stopTimer.Reset();

		UpdateControls();
		return;
	}

	shouldStopSequence = false; //Reset var
	DeleteEntity(&item); //Force delete old item
	return;
}
#pragma endregion

//////////////////////////////////SHINE TORCH//////////////////////////////
#pragma region Shine Torch
static constexpr int torchHash = 0xC9F33B0;		//Prop_CS_Police_Torch_02							Prop_CS_Police_Torch
static constexpr char* torchBaseAnimDict = "amb@world_human_security_shine_torch@male@base";
static constexpr char* torchBaseAnim = "base";

static constexpr char* torchEnterAnimDict = "amb@world_human_security_shine_torch@male@enter";
static constexpr char* torchEnterAnim = "enter";

static constexpr char* torchExitAnimDict = "amb@world_human_security_shine_torch@male@exit";
static constexpr char* torchExitAnim = "exit";

void cShineTorchSequence::StopAllAnims()
{
	StopAnimTask(GetPlayerPed(), torchEnterAnimDict, torchEnterAnim);
	StopAnimTask(GetPlayerPed(), torchBaseAnimDict, torchBaseAnim);
	StopAnimTask(GetPlayerPed(), torchExitAnimDict, torchExitAnim);
	REMOVE_ANIM_DICT(torchEnterAnimDict);
	REMOVE_ANIM_DICT(torchBaseAnimDict);
	REMOVE_ANIM_DICT(torchExitAnimDict);
	return;
}

void cShineTorchSequence::PlaySequence()
{
	const int leftHandID = GET_PED_BONE_INDEX(GetPlayerPed(), BONETAG_PH_L_HAND);

	SetPlayerControls(); //Player control should be disabled here and not during the sequence

	switch (sequenceState)
	{
	case INITIALIZED:
		item = CreateObject(torchHash);
		SET_ENTITY_AS_MISSION_ENTITY(item, false, true);
		ATTACH_ENTITY_TO_ENTITY(item, GetPlayerPed(), leftHandID, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 90.0f, true, true, false, false, 2, true);
		SET_ENTITY_VISIBLE(item, false, false);

		PlayAnimAndWait(torchEnterAnimDict, torchEnterAnim, upperSecondaryAF, LOOP, 0.0f, SLOW_BLEND_IN, SLOWEST_BLEND_OUT);
		break;
	case WAITING_FOR_ANIMATION_TO_END:
		if (!IS_ENTITY_PLAYING_ANIM(GetPlayerPed(), lastAnimDict, lastAnim, 3) ||
			HAS_ENTITY_ANIM_FINISHED(GetPlayerPed(), lastAnimDict, lastAnim, 3))
			sequenceState = nextSequenceState;

		if (GET_ENTITY_ANIM_CURRENT_TIME(GetPlayerPed(), torchEnterAnimDict, torchEnterAnim) > 0.35f)
			SET_ENTITY_VISIBLE(item, true, false);

		if (GET_ENTITY_ANIM_CURRENT_TIME(GetPlayerPed(), torchExitAnimDict, torchExitAnim) > 0.55f)
			DeleteEntity(&item);
		break;
	case LOOP:
		if (!IS_ENTITY_PLAYING_ANIM(GetPlayerPed(), torchBaseAnimDict, torchBaseAnim, 3))
			PlayAnimTask(GetPlayerPed(), torchBaseAnimDict, torchBaseAnim, upperSecondaryAF | AF_LOOPING, 0.0f, INSTANT_BLEND_IN, SLOWEST_BLEND_OUT);

		Vector3 loc = GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(item, 0.0f, -0.21f, 0.0f);
		Vector3 dir = GET_ENTITY_FORWARD_VECTOR(item); dir.x *= -1.0f; dir.y *= -1.0f; dir.z *= -1.0f;
		DRAW_SHADOWED_SPOT_LIGHT(loc.x, loc.y, loc.z, dir.x, dir.y, dir.z, 255, 255, 255, 7.5f, 0.25f, 20.0f, 25.0f, 7.5f, 0);
		break;
	case EXITING:
		PlayAnimAndWait(torchExitAnimDict, torchExitAnim, upperSecondaryAF, FLUSH_ASSETS, 0.0f, INSTANT_BLEND_IN);
		break;
	case FLUSH_ASSETS:
		DeleteEntity(&item);
		SET_MODEL_AS_NO_LONGER_NEEDED(torchHash);
		REMOVE_ANIM_DICT(torchEnterAnimDict);
		REMOVE_ANIM_DICT(torchBaseAnimDict);
		REMOVE_ANIM_DICT(torchExitAnimDict);
		shouldStopSequence = false;
		sequenceState = FINISHED;
		break;
	case STREAM_ASSETS_IN:
		if (RequestModel(torchHash) && RequestAnimDict(torchEnterAnimDict) &&
			RequestAnimDict(torchBaseAnimDict) && RequestAnimDict(torchExitAnimDict))
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

void cShineTorchSequence::SetState(int state)
{
	if (IS_ENTITY_PLAYING_ANIM(GetPlayerPed(), torchEnterAnimDict, torchEnterAnim, 3) ||
		IS_ENTITY_PLAYING_ANIM(GetPlayerPed(), torchExitAnimDict, torchExitAnim, 3))
		return;

	if (state == EXITING && sequenceState != WAITING_FOR_ANIMATION_TO_END && sequenceState != EXITING && sequenceState != FLUSH_ASSETS && sequenceState != FINISHED)
		sequenceState = EXITING;
	else if (state != EXITING && state != sequenceState)
		sequenceState = state;

	return;
}

void cShineTorchSequence::UpdateControls()
{
	if (sequenceState != LOOP)
		return;

	if (instructionalButtonsText != NULL)
	{
		AddScaleformInstructionalButton(control, input, instructionalButtonsText, true);
		RunScaleformInstructionalButtons();
	}

	if (IS_DISABLED_CONTROL_JUST_PRESSED(control, input))
	{
		controlTimer.Reset();
		return;
	}

	if (controlTimer.Get() > holdTime && IS_DISABLED_CONTROL_PRESSED(control, input))
		shouldStopSequence = true;

	return;
}

void cShineTorchSequence::ForceStop()
{
	if (sequenceState == FLUSH_ASSETS || sequenceState == FINISHED)
		return;

	StopAllAnims();
	DeleteEntity(&item);
	shouldStopSequence = false;
	sequenceState = FLUSH_ASSETS;
	PlaySequence();
	return;
}

void cShineTorchSequence::Update()
{
	if (sequenceState != FINISHED)
	{
		if (!AdditionalChecks(GetPlayerPed()))
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
			stopTimer.Reset();

		UpdateControls();
		return;
	}

	shouldStopSequence = false; //Reset var
	DeleteEntity(&item); //Force delete old item
	return;
}
#pragma endregion

//////////////////////////////////LIFT CURL BAR/////////////////////////////
#pragma region Lift Curl Bar
static constexpr int curlBarHash = 0xA345B107;		//Prop_Curl_bar_01
static constexpr char* curlBarBaseAnimDict = "amb@world_human_muscle_free_weights@male@barbell@base";
static constexpr char* curlBarBaseAnim = "base";
static constexpr char* curlBarIdleAnimDict = "amb@world_human_muscle_free_weights@male@barbell@idle_a";
static constexpr char* curlBarIdleAnim = "idle_a";

void cLiftCurlBarSequence::StopAllAnims()
{
	StopAnimTask(GetPlayerPed(), curlBarBaseAnimDict, curlBarBaseAnim);
	StopAnimTask(GetPlayerPed(), curlBarIdleAnimDict, curlBarIdleAnim);
	REMOVE_ANIM_DICT(curlBarBaseAnimDict);
	REMOVE_ANIM_DICT(curlBarIdleAnimDict);
	return;
}

void cLiftCurlBarSequence::PlaySequence()
{
	const int rightHandID = GET_PED_BONE_INDEX(GetPlayerPed(), BONETAG_PH_R_HAND);

	SetPlayerControls(); //Player control should be disabled here and not during the sequence

	switch (sequenceState)
	{
	case INITIALIZED:
		item = CreateObject(curlBarHash);
		SET_ENTITY_AS_MISSION_ENTITY(item, false, true);
		ATTACH_ENTITY_TO_ENTITY(item, GetPlayerPed(), rightHandID, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, true, true, false, false, 2, true);

		PlayAnimAndWait(curlBarBaseAnimDict, curlBarBaseAnim, upperSecondaryAF | AF_HOLD_LAST_FRAME, HOLD, 0.0f, SLOW_BLEND_IN, SLOWEST_BLEND_OUT);
		break;
	case WAITING_FOR_ANIMATION_TO_END:
		if (!IS_ENTITY_PLAYING_ANIM(GetPlayerPed(), lastAnimDict, lastAnim, 3) ||
			HAS_ENTITY_ANIM_FINISHED(GetPlayerPed(), lastAnimDict, lastAnim, 3))
			sequenceState = nextSequenceState;
		break;
	case LIFT:
		if (!IS_ENTITY_PLAYING_ANIM(GetPlayerPed(), curlBarBaseAnimDict, curlBarBaseAnim, 3))
			PlayAnimAndWait(curlBarBaseAnimDict, curlBarBaseAnim, upperSecondaryAF | AF_HOLD_LAST_FRAME, HOLD, 0.0f, INSTANT_BLEND_IN, SLOWEST_BLEND_OUT);
		break;
	case HOLD:
		if (!IS_ENTITY_PLAYING_ANIM(GetPlayerPed(), curlBarIdleAnimDict, curlBarIdleAnim, 3))
			PlayAnimTask(GetPlayerPed(), curlBarIdleAnimDict, curlBarIdleAnim, upperSecondaryAF | AF_LOOPING, 0.0f, INSTANT_BLEND_IN, SLOWEST_BLEND_OUT);
		break;
	case EXITING:
		StopAllAnims();
		sequenceState = FLUSH_ASSETS;
		break;
	case FLUSH_ASSETS:
		DETACH_ENTITY(item, false, false);
		SET_ENTITY_AS_NO_LONGER_NEEDED(&item);
		SET_MODEL_AS_NO_LONGER_NEEDED(curlBarHash);

		REMOVE_ANIM_DICT(curlBarBaseAnimDict);
		REMOVE_ANIM_DICT(curlBarIdleAnimDict);
		shouldStopSequence = false;
		sequenceState = FINISHED;
		break;
	case STREAM_ASSETS_IN:
		if (RequestModel(curlBarHash) && RequestAnimDict(curlBarBaseAnimDict) && RequestAnimDict(curlBarIdleAnimDict))
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

void cLiftCurlBarSequence::SetState(int state)
{
	if (IS_ENTITY_PLAYING_ANIM(GetPlayerPed(), curlBarBaseAnimDict, curlBarBaseAnim, 3))
		return;

	if (state == EXITING && sequenceState != WAITING_FOR_ANIMATION_TO_END && sequenceState != EXITING && sequenceState != FLUSH_ASSETS && sequenceState != FINISHED)
		sequenceState = EXITING;
	else if (state != EXITING && state != sequenceState)
		sequenceState = state;

	return;
}

void cLiftCurlBarSequence::UpdateControls()
{
	if (sequenceState != HOLD)
		return;

	if (instructionalButtonsText != NULL)
	{
		AddScaleformInstructionalButton(control, input, instructionalButtonsText, true);
		RunScaleformInstructionalButtons();
	}

	if (IS_DISABLED_CONTROL_JUST_PRESSED(control, input))
	{
		controlTimer.Reset();
		return;
	}

	if (controlTimer.Get() < holdTime)
	{
		if (IS_DISABLED_CONTROL_JUST_RELEASED(control, input))
			SetState(LIFT);
	}
	else if (IS_DISABLED_CONTROL_PRESSED(control, input))
		shouldStopSequence = true;

	return;
}

void cLiftCurlBarSequence::ForceStop()
{
	if (sequenceState == FLUSH_ASSETS || sequenceState == FINISHED)
		return;

	StopAllAnims();
	shouldStopSequence = false;
	sequenceState = FLUSH_ASSETS;
	PlaySequence();
	DeleteEntity(&item);
	return;
}

void cLiftCurlBarSequence::Update()
{
	if (sequenceState != FINISHED)
	{
		if (!AdditionalChecks(GetPlayerPed()))
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
			stopTimer.Reset();

		UpdateControls();
		return;
	}

	shouldStopSequence = false; //Reset var
	return;
}
#pragma endregion

//////////////////////////////////BINOCULARS//////////////////////////////
#pragma region Binoculars
static constexpr int binocularsHash = 0x3AB773DB;		//Prop_binoc_01
static constexpr char* binocularsBaseAnimDict = "amb@world_human_binoculars@male@base";
static constexpr char* binocularsBaseAnim = "base";

static constexpr char* binocularsEnterAnimDict = "amb@world_human_binoculars@male@enter";
static constexpr char* binocularsEnterAnim = "enter";

static constexpr char* binocularsExitAnimDict = "amb@world_human_binoculars@male@exit";
static constexpr char* binocularsExitAnim = "exit";

//Can be used as weapon movement with custom clipset "amb@world_human_binoculars_male@base"

void cBinocularsSequence::StopAllAnims()
{
	StopAnimTask(GetPlayerPed(), binocularsEnterAnimDict, binocularsEnterAnim);
	StopAnimTask(GetPlayerPed(), binocularsBaseAnimDict, binocularsBaseAnim);
	StopAnimTask(GetPlayerPed(), binocularsExitAnimDict, binocularsExitAnim);
	REMOVE_ANIM_DICT(binocularsEnterAnimDict);
	REMOVE_ANIM_DICT(binocularsBaseAnimDict);
	REMOVE_ANIM_DICT(binocularsExitAnimDict);
	return;
}

void cBinocularsSequence::PlaySequence()
{
	const int rightHandID = GET_PED_BONE_INDEX(GetPlayerPed(), BONETAG_PH_R_HAND);

	SetPlayerControls(); //Player control should be disabled here and not during the sequence

	switch (sequenceState)
	{
	case INITIALIZED:
		item = CreateObject(binocularsHash);
		SET_ENTITY_AS_MISSION_ENTITY(item, false, true);
		ATTACH_ENTITY_TO_ENTITY(item, GetPlayerPed(), rightHandID, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, true, true, false, false, 2, true);

		PlayAnimAndWait(binocularsEnterAnimDict, binocularsEnterAnim, upperSecondaryAF | AF_HOLD_LAST_FRAME, LOOP, 0.0f, SLOW_BLEND_IN, SLOWEST_BLEND_OUT);
		break;
	case WAITING_FOR_ANIMATION_TO_END:
		if (!IS_ENTITY_PLAYING_ANIM(GetPlayerPed(), lastAnimDict, lastAnim, 3) ||
			HAS_ENTITY_ANIM_FINISHED(GetPlayerPed(), lastAnimDict, lastAnim, 3))
			sequenceState = nextSequenceState;

		if (GET_ENTITY_ANIM_CURRENT_TIME(GetPlayerPed(), binocularsExitAnimDict, binocularsExitAnim) > 0.9f)
			DeleteEntity(&item);
		break;
	case LOOP:
		if (!IS_ENTITY_PLAYING_ANIM(GetPlayerPed(), binocularsBaseAnimDict, binocularsBaseAnim, 3))
		{
			PlayAnimTask(GetPlayerPed(), binocularsBaseAnimDict, binocularsBaseAnim, upperSecondaryAF | AF_LOOPING, 0.0f, INSTANT_BLEND_IN, SLOWEST_BLEND_OUT);
			TASK_LOOK_AT_ENTITY(GetPlayerPed(), item, -1, SLF_NARROW_YAW_LIMIT | SLF_NARROW_PITCH_LIMIT | SLF_SLOW_TURN_RATE, 2);
		}
		break;
	case EXITING:
		PlayAnimAndWait(binocularsExitAnimDict, binocularsExitAnim, upperSecondaryAF, FLUSH_ASSETS, 0.0f, INSTANT_BLEND_IN);
		TASK_CLEAR_LOOK_AT(GetPlayerPed());
		break;
	case FLUSH_ASSETS:
		DeleteEntity(&item);
		SET_MODEL_AS_NO_LONGER_NEEDED(binocularsHash);
		REMOVE_ANIM_DICT(binocularsEnterAnimDict);
		REMOVE_ANIM_DICT(binocularsBaseAnimDict);
		REMOVE_ANIM_DICT(binocularsExitAnimDict);
		shouldStopSequence = false;
		sequenceState = FINISHED;
		break;
	case STREAM_ASSETS_IN:
		if (RequestModel(binocularsHash) && RequestAnimDict(binocularsEnterAnimDict) &&
			RequestAnimDict(binocularsBaseAnimDict) && RequestAnimDict(binocularsExitAnimDict))
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

void cBinocularsSequence::SetState(int state)
{
	if (IS_ENTITY_PLAYING_ANIM(GetPlayerPed(), binocularsEnterAnimDict, binocularsEnterAnim, 3) ||
		IS_ENTITY_PLAYING_ANIM(GetPlayerPed(), binocularsExitAnimDict, binocularsExitAnim, 3))
		return;

	if (state == EXITING && sequenceState != WAITING_FOR_ANIMATION_TO_END && sequenceState != EXITING && sequenceState != FLUSH_ASSETS && sequenceState != FINISHED)
		sequenceState = EXITING;
	else if (state != EXITING && state != sequenceState)
		sequenceState = state;

	return;
}

void cBinocularsSequence::UpdateControls()
{
	if (sequenceState != LOOP)
		return;

	if (instructionalButtonsText != NULL)
	{
		AddScaleformInstructionalButton(control, input, instructionalButtonsText, true);
		RunScaleformInstructionalButtons();
	}

	if (IS_DISABLED_CONTROL_JUST_PRESSED(control, input))
	{
		controlTimer.Reset();
		return;
	}

	if (controlTimer.Get() > holdTime && IS_DISABLED_CONTROL_PRESSED(control, input))
		shouldStopSequence = true;

	return;
}

void cBinocularsSequence::ForceStop()
{
	if (sequenceState == FLUSH_ASSETS || sequenceState == FINISHED)
		return;

	StopAllAnims();
	TASK_CLEAR_LOOK_AT(GetPlayerPed());
	DeleteEntity(&item);
	shouldStopSequence = false;
	sequenceState = FLUSH_ASSETS;
	PlaySequence();
	return;
}

void cBinocularsSequence::Update()
{
	if (sequenceState != FINISHED)
	{
		if (!AdditionalChecks(GetPlayerPed()))
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
			stopTimer.Reset();

		UpdateControls();
		return;
	}

	shouldStopSequence = false; //Reset var
	DeleteEntity(&item); //Force delete old item
	return;
}
#pragma endregion

//////////////////////////////////HOLD BUM SIGN/////////////////////////////
#pragma region Hold Bum Sign
static constexpr char* bumSignAnimDict = "amb@world_human_bum_freeway@male@base";
static constexpr char* bumSignBaseAnim = "base";

int cHoldBumSignSequence::GetBumSignPropHash() const
{
	switch (GetRandomIntInRange(0, 3))
	{
	case 0:
		return 0xF15FB3DD; break;	//Prop_Beggers_Sign_01
	case 1:
		return 0xE0311180; break;	//Prop_Beggers_Sign_02
	case 2:
		return 0xBDE0CCD4; break;	//Prop_Beggers_Sign_03
	case 3:
		return 0xD0357169; break;	//Prop_Beggers_Sign_04
	}

	return 0xF15FB3DD;
}

void cHoldBumSignSequence::StopAllAnims()
{
	StopAnimTask(GetPlayerPed(), bumSignAnimDict, bumSignBaseAnim);
	REMOVE_ANIM_DICT(bumSignAnimDict);
	return;
}

void cHoldBumSignSequence::PlaySequence()
{
	const int rightHandID = GET_PED_BONE_INDEX(GetPlayerPed(), BONETAG_PH_R_HAND);

	SetPlayerControls(); //Player control should be disabled here and not during the sequence

	switch (sequenceState)
	{
	case INITIALIZED:
		item = CreateObject(itemHash);
		SET_ENTITY_AS_MISSION_ENTITY(item, false, true);
		ATTACH_ENTITY_TO_ENTITY(item, GetPlayerPed(), rightHandID, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, true, true, false, false, 2, true);
		PlayAnimTask(GetPlayerPed(), bumSignAnimDict, bumSignBaseAnim, upperSecondaryAF | AF_LOOPING);
		sequenceState = LOOP;
		break;
	case WAITING_FOR_ANIMATION_TO_END:
		if (!IS_ENTITY_PLAYING_ANIM(GetPlayerPed(), lastAnimDict, lastAnim, 3) ||
			HAS_ENTITY_ANIM_FINISHED(GetPlayerPed(), lastAnimDict, lastAnim, 3))
			sequenceState = nextSequenceState;
		break;
	case LOOP:
		break;
	case EXITING:
		StopAllAnims();
		sequenceState = FLUSH_ASSETS;
		break;
	case FLUSH_ASSETS:
		REMOVE_ANIM_DICT(bumSignAnimDict);
		DETACH_ENTITY(item, false, false);
		SET_ENTITY_AS_NO_LONGER_NEEDED(&item);
		SET_MODEL_AS_NO_LONGER_NEEDED(itemHash);
		itemHash = NULL;
		shouldStopSequence = false;
		sequenceState = FINISHED;
		break;
	case STREAM_ASSETS_IN:
		if (itemHash == NULL)
			itemHash = GetBumSignPropHash();

		if (RequestModel(itemHash) && RequestAnimDict(bumSignAnimDict))
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

void cHoldBumSignSequence::SetState(int state)
{
	StopAnimTask(GetPlayerPed(), bumSignAnimDict, bumSignBaseAnim);

	if (state == EXITING && sequenceState != WAITING_FOR_ANIMATION_TO_END && sequenceState != EXITING && sequenceState != FLUSH_ASSETS && sequenceState != FINISHED)
		sequenceState = EXITING;
	else if (state != EXITING && state != sequenceState)
		sequenceState = state;

	return;
}

void cHoldBumSignSequence::UpdateControls()
{
	if (sequenceState != LOOP)
		return;

	if (instructionalButtonsText != NULL)
	{
		AddScaleformInstructionalButton(control, input, instructionalButtonsText, true);
		RunScaleformInstructionalButtons();
	}

	if (IS_DISABLED_CONTROL_JUST_PRESSED(control, input))
	{
		controlTimer.Reset();
		return;
	}

	if (controlTimer.Get() > holdTime && IS_DISABLED_CONTROL_PRESSED(control, input))
		shouldStopSequence = true;

	return;
}

void cHoldBumSignSequence::ForceStop()
{
	if (sequenceState == FLUSH_ASSETS || sequenceState == FINISHED)
		return;

	StopAllAnims();
	shouldStopSequence = false;
	sequenceState = FLUSH_ASSETS;
	PlaySequence();
	DeleteEntity(&item);
	return;
}

void cHoldBumSignSequence::Update()
{
	if (sequenceState != FINISHED)
	{
		if (!AdditionalChecks(GetPlayerPed()))
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
			stopTimer.Reset();

		UpdateControls();
		return;
	}

	shouldStopSequence = false; //Reset var
	return;
}
#pragma endregion

//////////////////////////////////FISHING/////////////////////////////
#pragma region Fishing
static constexpr int fishingRodHash = 0x8E1E7CCF;		//Prop_Fishing_Rod_01
static constexpr char* fishingAnimDict = "amb@world_human_stand_fishing@base";
static constexpr char* fishingBaseAnim = "base";

void cFishingSequence::StopAllAnims()
{
	StopAnimTask(GetPlayerPed(), fishingAnimDict, fishingBaseAnim);
	REMOVE_ANIM_DICT(fishingAnimDict);
	return;
}

void cFishingSequence::PlaySequence()
{
	const int leftHandID = GET_PED_BONE_INDEX(GetPlayerPed(), BONETAG_PH_L_HAND);

	SetPlayerControls(); //Player control should be disabled here and not during the sequence

	switch (sequenceState)
	{
	case INITIALIZED:
		item = CreateObject(fishingRodHash);
		SET_ENTITY_AS_MISSION_ENTITY(item, false, true);
		ATTACH_ENTITY_TO_ENTITY(item, GetPlayerPed(), leftHandID, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, true, true, false, false, 2, true);
		PlayAnimTask(GetPlayerPed(), fishingAnimDict, fishingBaseAnim, upperSecondaryAF | AF_LOOPING);
		sequenceState = LOOP;
		break;
	case WAITING_FOR_ANIMATION_TO_END:
		if (!IS_ENTITY_PLAYING_ANIM(GetPlayerPed(), lastAnimDict, lastAnim, 3) ||
			HAS_ENTITY_ANIM_FINISHED(GetPlayerPed(), lastAnimDict, lastAnim, 3))
			sequenceState = nextSequenceState;
		break;
	case LOOP:
		break;
	case EXITING:
		StopAllAnims();
		sequenceState = FLUSH_ASSETS;
		break;
	case FLUSH_ASSETS:
		DETACH_ENTITY(item, false, false);
		SET_ENTITY_AS_NO_LONGER_NEEDED(&item);
		SET_MODEL_AS_NO_LONGER_NEEDED(fishingRodHash);

		REMOVE_ANIM_DICT(fishingAnimDict);
		shouldStopSequence = false;
		sequenceState = FINISHED;
		break;
	case STREAM_ASSETS_IN:
		if (RequestModel(fishingRodHash) && RequestAnimDict(fishingAnimDict))
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

void cFishingSequence::SetState(int state)
{
	StopAnimTask(GetPlayerPed(), fishingAnimDict, fishingBaseAnim);

	if (state == EXITING && sequenceState != WAITING_FOR_ANIMATION_TO_END && sequenceState != EXITING && sequenceState != FLUSH_ASSETS && sequenceState != FINISHED)
		sequenceState = EXITING;
	else if (state != EXITING && state != sequenceState)
		sequenceState = state;

	return;
}

void cFishingSequence::UpdateControls()
{
	if (sequenceState != LOOP)
		return;

	if (instructionalButtonsText != NULL)
	{
		AddScaleformInstructionalButton(control, input, instructionalButtonsText, true);
		RunScaleformInstructionalButtons();
	}

	if (IS_DISABLED_CONTROL_JUST_PRESSED(control, input))
	{
		controlTimer.Reset();
		return;
	}

	if (controlTimer.Get() > holdTime && IS_DISABLED_CONTROL_PRESSED(control, input))
		shouldStopSequence = true;

	return;
}

void cFishingSequence::ForceStop()
{
	if (sequenceState == FLUSH_ASSETS || sequenceState == FINISHED)
		return;

	StopAllAnims();
	shouldStopSequence = false;
	sequenceState = FLUSH_ASSETS;
	PlaySequence();
	DeleteEntity(&item);
	return;
}

void cFishingSequence::Update()
{
	if (sequenceState != FINISHED)
	{
		if (!AdditionalChecks(GetPlayerPed()))
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
			stopTimer.Reset();

		UpdateControls();
		return;
	}

	shouldStopSequence = false; //Reset var
	return;
}
#pragma endregion
