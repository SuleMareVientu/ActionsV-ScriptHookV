//ScriptHook
#include <natives.h>
// #include <types.h> //Already included in globals.h
//Custom
#include "functions.h"
#include "..\script.h"

//Custom implementation of TIMERA and TIMERB natives
#pragma region Timer
void Timer::Set(int value)
{
	gameTimer = GET_GAME_TIMER() + value;
	return;
}

int Timer::Get() const
{
	return (GET_GAME_TIMER() - gameTimer);
}
#pragma endregion

#pragma region Ped Flags
void EnablePedConfigFlag(Ped ped, int flag)
{
	if (!PED::GET_PED_CONFIG_FLAG(ped, flag, false))
		PED::SET_PED_CONFIG_FLAG(ped, flag, true);
	return;
}

void DisablePedConfigFlag(Ped ped, int flag)
{
	if (PED::GET_PED_CONFIG_FLAG(ped, flag, false))
		PED::SET_PED_CONFIG_FLAG(ped, flag, false);
	return;
}

void EnablePedResetFlag(Ped ped, int flag)
{
	if (!PED::GET_PED_RESET_FLAG(ped, flag))
		PED::SET_PED_RESET_FLAG(ped, flag, true);
	return;
}

void DisablePedResetFlag(Ped ped, int flag)
{
	if (PED::GET_PED_RESET_FLAG(ped, flag))
		PED::SET_PED_RESET_FLAG(ped, flag, false);
	return;
}
#pragma endregion

#pragma region Print
void Print(char* string, int ms)
{
	BEGIN_TEXT_COMMAND_PRINT("STRING");
	ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME(string);
	END_TEXT_COMMAND_PRINT(ms, 1);
	return;
}

void PrintInt(int value, int ms)
{
	BEGIN_TEXT_COMMAND_PRINT("NUMBER");
	ADD_TEXT_COMPONENT_INTEGER(value);
	END_TEXT_COMMAND_PRINT(ms, 1);
	return;
}

void PrintFloat(float value, int ms)
{
	BEGIN_TEXT_COMMAND_PRINT("NUMBER");
	ADD_TEXT_COMPONENT_FLOAT(value, 4);
	END_TEXT_COMMAND_PRINT(ms, 1);
	return;
}

void PrintHelp(char* string, bool playSound, int overrideDuration)
{
	BEGIN_TEXT_COMMAND_DISPLAY_HELP("STRING");
	ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME(string);
	END_TEXT_COMMAND_DISPLAY_HELP(NULL, false, playSound, overrideDuration);
	return;
}
#pragma endregion

#pragma region Streaming
bool RequestModel(Hash model)
{
	if (!IS_MODEL_VALID(model))
		return false;

	if (!HAS_MODEL_LOADED(model))
	{
		REQUEST_MODEL(model);
		return false;
	}
	return true;
}

Object CreateObject(Hash model, float locX, float locY, float locZ, float rotX, float rotY, float rotZ)
{
	Object obj = CREATE_OBJECT_NO_OFFSET(model, locX, locY, locZ, false, false, false);
	if (rotX != NULL || rotY != NULL || rotZ != NULL)
		SET_ENTITY_ROTATION(obj, rotX, rotY, rotZ, 2, false);

	return obj;
}

void DeleteEntity(Entity* obj)
{
	if (DOES_ENTITY_EXIST(*obj))
	{
		if (ENTITY::IS_ENTITY_ATTACHED(*obj))
			ENTITY::DETACH_ENTITY(*obj, false, false);

		SET_ENTITY_AS_MISSION_ENTITY(*obj, false, true);
		DELETE_ENTITY(obj);
	}
	return;
}

bool RequestAnimDict(char* animDict)
{
	if (!DOES_ANIM_DICT_EXIST(animDict))
		return false;

	if (!HAS_ANIM_DICT_LOADED(animDict))
	{
		REQUEST_ANIM_DICT(animDict);
		return false;
	}
	return true;
}

bool StopAnimTask(Entity entity, const char* animDict, const char* animName, float blendDelta)
{
	if (IS_ENTITY_PLAYING_ANIM(entity, animDict, animName, 3))
	{
		STOP_ANIM_TASK(entity, animDict, animName, blendDelta);
		return true;
	}

	return false;
}

bool RequestClipSet(char* animSet)
{
	if (!HAS_CLIP_SET_LOADED(animSet))
	{
		REQUEST_CLIP_SET(animSet);
		return false;
	}
	return true;
}

bool RequestAudioBank(char* bank)
{
	if (!REQUEST_SCRIPT_AUDIO_BANK(bank, false, -1))
		return false;

	return true;
}

bool StopAudioStream()
{
	if (IS_STREAM_PLAYING())
	{
		STOP_STREAM();
		return true;
	}

	return false;
}

bool RequestPTFXAsset(char* asset)
{
	if (!HAS_NAMED_PTFX_ASSET_LOADED(asset))
	{
		REQUEST_NAMED_PTFX_ASSET(asset);
		return false;
	}
	return true;
}
#pragma endregion

#pragma region Particle Effects
bool StartParticleFxNonLoopedOnEntity(const char* PTFXAsset, const char* effectName, Entity entity, float scale, float offsetX, float offsetY, float offsetZ, float rotX, float rotY, float rotZ, bool axisX, bool axisY, bool axisZ)
{
	USE_PARTICLE_FX_ASSET(PTFXAsset);
	return START_PARTICLE_FX_NON_LOOPED_ON_ENTITY(effectName, entity, offsetX, offsetY, offsetZ, rotX, rotY, rotZ, scale, axisX, axisY, axisZ);
}

int StartParticleFxLoopedOnPedBone(const char* PTFXAsset, const char* effectName, Ped ped, int boneIndex, float scale, float xOffset, float yOffset, float zOffset, float xRot, float yRot, float zRot, bool xAxis, bool yAxis, bool zAxis)
{
	USE_PARTICLE_FX_ASSET(PTFXAsset);
	return START_PARTICLE_FX_LOOPED_ON_PED_BONE(effectName, ped, xOffset, yOffset, zOffset, xRot, yRot, zRot, boneIndex, scale, xAxis, yAxis, zAxis);
}

bool StartParticleFxNonLoopedOnPedBone(const char* PTFXAsset, const char* effectName, Ped ped, int boneIndex, float scale, float offsetX, float offsetY, float offsetZ, float rotX, float rotY, float rotZ, bool axisX, bool axisY, bool axisZ)
{
	USE_PARTICLE_FX_ASSET(PTFXAsset);
	return START_PARTICLE_FX_NON_LOOPED_ON_PED_BONE(effectName, ped, offsetX, offsetY, offsetZ, rotX, rotY, rotZ, boneIndex, scale, axisX, axisY, axisZ);
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
#pragma endregion

#pragma region Player Actions Control
bool AdditionalChecks(Ped ped, bool countEnemies)
{
	if (!DOES_ENTITY_EXIST(ped) ||
		IS_ENTITY_DEAD(ped, false) ||
		IS_PED_DEAD_OR_DYING(ped, true) ||
		IS_PED_INJURED(ped) ||
		IS_PED_USING_ANY_SCENARIO(ped) ||
		IS_PED_RAGDOLL(ped) ||
		IS_PED_GETTING_UP(ped) ||
		IS_PED_FALLING(ped) ||
		IS_PED_JUMPING(ped) ||
		IS_PED_DIVING(ped) ||
		IS_PED_SWIMMING(ped) ||
		IS_PED_GOING_INTO_COVER(ped) ||
		IS_PED_CLIMBING(ped) ||
		IS_PED_VAULTING(ped) ||
		IS_PED_HANGING_ON_TO_VEHICLE(ped) ||
		IS_PED_IN_ANY_VEHICLE(ped, true) ||
		IS_PED_IN_COVER(ped, false) ||
		!IS_PED_ON_FOOT(ped) ||
		IS_PED_TAKING_OFF_HELMET(ped) ||
		GET_ENTITY_SUBMERGED_LEVEL(ped) >= 0.7f ||
		(countEnemies && (IS_PED_IN_MELEE_COMBAT(ped) || COUNT_PEDS_IN_COMBAT_WITH_TARGET(ped) > 0)))
		return false;

	return true;
}

void DisablePlayerActionsThisFrame(bool canSprint, float maxMoveBlendRatio)
{
	Ped ped = PLAYER_PED_ID();
	SET_PED_RESET_FLAG(ped, PRF_DisablePlayerJumping, true);
	SET_PED_RESET_FLAG(ped, PRF_DisablePlayerVaulting, true);

	if (!canSprint)
	{
		DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SPRINT, false);
		SET_PED_MAX_MOVE_BLEND_RATIO(ped, PEDMOVEBLENDRATIO_WALK);	//Prevents the player from running/sprinting
	}
	else
		SET_PED_MAX_MOVE_BLEND_RATIO(ped, maxMoveBlendRatio);

	if (IS_SCRIPTED_CONVERSATION_ONGOING() || IS_MOBILE_PHONE_CALL_ONGOING())
		STOP_SCRIPTED_CONVERSATION(false);

	if (IS_PED_RINGTONE_PLAYING(ped))
		STOP_PED_RINGTONE(ped);

	DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_CELLPHONE_UP, false);
	DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_CELLPHONE_DOWN, false);
	DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_CELLPHONE_LEFT, false);
	DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_CELLPHONE_RIGHT, false);
	DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_CELLPHONE_SELECT, false);
	DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_CELLPHONE_CANCEL, false);
	DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_CELLPHONE_OPTION, false);
	DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_CELLPHONE_EXTRA_OPTION, false);
	DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_CELLPHONE_SCROLL_FORWARD, false);
	DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_CELLPHONE_SCROLL_BACKWARD, false);
	DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_CELLPHONE_CAMERA_FOCUS_LOCK, false);
	DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_CELLPHONE_CAMERA_GRID, false);
	DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_CELLPHONE_CAMERA_SELFIE, false);
	DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_CELLPHONE_CAMERA_DOF, false);
	DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_CELLPHONE_CAMERA_EXPRESSION, false);

	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_TALK, true);	//Disables talking
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_WEAPON_WHEEL_UD, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_WEAPON_WHEEL_LR, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_WEAPON_WHEEL_NEXT, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_WEAPON_WHEEL_PREV, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SELECT_NEXT_WEAPON, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SELECT_PREV_WEAPON, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_CHARACTER_WHEEL, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_MULTIPLAYER_INFO, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_JUMP, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_ENTER, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_ATTACK, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_AIM, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_PHONE, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SPECIAL_ABILITY, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SPECIAL_ABILITY_SECONDARY, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_DUCK, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SELECT_WEAPON, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_PICKUP, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SNIPER_ZOOM, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SNIPER_ZOOM_IN_ONLY, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SNIPER_ZOOM_OUT_ONLY, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SNIPER_ZOOM_IN_SECONDARY, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SNIPER_ZOOM_OUT_SECONDARY, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_COVER, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_RELOAD, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_DETONATE, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_HUD_SPECIAL, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_ARREST, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_ACCURATE_AIM, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_CONTEXT, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_CONTEXT_SECONDARY, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_WEAPON_SPECIAL, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_WEAPON_SPECIAL_TWO, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_DIVE, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_DROP_WEAPON, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_DROP_AMMO, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_THROW_GRENADE, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_VEH_EXIT, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_MELEE_ATTACK_LIGHT, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_MELEE_ATTACK_HEAVY, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_MELEE_ATTACK_ALTERNATE, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_MELEE_BLOCK, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SELECT_WEAPON_UNARMED, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SELECT_WEAPON_MELEE, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SELECT_WEAPON_HANDGUN, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SELECT_WEAPON_SHOTGUN, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SELECT_WEAPON_SMG, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SELECT_WEAPON_AUTO_RIFLE, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SELECT_WEAPON_SNIPER, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SELECT_WEAPON_HEAVY, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SELECT_WEAPON_SPECIAL, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SELECT_CHARACTER_MICHAEL, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SELECT_CHARACTER_FRANKLIN, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SELECT_CHARACTER_TREVOR, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SELECT_CHARACTER_MULTIPLAYER, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SPECIAL_ABILITY_PC, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_INTERACTION_MENU, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_ATTACK2, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_PREV_WEAPON, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_NEXT_WEAPON, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_MELEE_ATTACK1, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_MELEE_ATTACK2, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SNIPER_ZOOM_IN, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SNIPER_ZOOM_OUT, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SNIPER_ZOOM_IN_ALTERNATE, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SNIPER_ZOOM_OUT_ALTERNATE, false);
	return;
}

void DisablePlayerControlThisFrame()
{
	//DisablePlayerActionsThisFrame(ped);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_MOVE_LR, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_MOVE_UD, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_MOVE_UP_ONLY, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_MOVE_DOWN_ONLY, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_MOVE_LEFT_ONLY, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_MOVE_RIGHT_ONLY, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_TALK, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_MOVE_LEFT, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_MOVE_RIGHT, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_MOVE_UP, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_MOVE_DOWN, false);
	return;
}
#pragma endregion

#pragma region Scaleforms
static ScaleformInstructionalButtons DisplayStruct;
static int InstructionalButtonsScaleformIndex = NULL;
void RunScaleformInstructionalButtons(bool refresh)
{
	if (!HAS_SCALEFORM_MOVIE_LOADED(InstructionalButtonsScaleformIndex))
	{
		InstructionalButtonsScaleformIndex = REQUEST_SCALEFORM_MOVIE("instructional_buttons");
		ScaleformInstructionalButtons tmp; DisplayStruct = tmp;	//Reset struct state
		return;
	}

	if (refresh == true || HAVE_CONTROLS_CHANGED(FRONTEND_CONTROL))
		DisplayStruct.bInitialised = false;

	if (!DisplayStruct.bInitialised)
	{

		if (!HAS_SCALEFORM_MOVIE_LOADED(InstructionalButtonsScaleformIndex))
			InstructionalButtonsScaleformIndex = REQUEST_SCALEFORM_MOVIE("instructional_buttons");
		else
		{
			CALL_SCALEFORM_MOVIE_METHOD(InstructionalButtonsScaleformIndex, "CLEAR_ALL");

			for (int i = 0; i < DisplayStruct.ButtonCount; i++) {
				BEGIN_SCALEFORM_MOVIE_METHOD(InstructionalButtonsScaleformIndex, "SET_DATA_SLOT");

				SCALEFORM_MOVIE_METHOD_ADD_PARAM_INT(i);

				SCALEFORM_MOVIE_METHOD_ADD_PARAM_PLAYER_NAME_STRING(GET_CONTROL_INSTRUCTIONAL_BUTTONS_STRING(DisplayStruct.Buttons[i].iButtonSlotControl, DisplayStruct.Buttons[i].iButtonSlotInput, true));

				BEGIN_TEXT_COMMAND_SCALEFORM_STRING("STRING");
				ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME(DisplayStruct.Buttons[i].sText);
				END_TEXT_COMMAND_SCALEFORM_STRING();

				END_SCALEFORM_MOVIE_METHOD();
			}

			BEGIN_SCALEFORM_MOVIE_METHOD(InstructionalButtonsScaleformIndex, "TOGGLE_MOUSE_BUTTONS");
			SCALEFORM_MOVIE_METHOD_ADD_PARAM_BOOL(false);
			END_SCALEFORM_MOVIE_METHOD();

			CALL_SCALEFORM_MOVIE_METHOD(InstructionalButtonsScaleformIndex, "DRAW_INSTRUCTIONAL_BUTTONS");
			DisplayStruct.bInitialised = true;
		}
	}

	HIDE_HUD_COMPONENT_THIS_FRAME(HUD_AREA_NAME);
	HIDE_HUD_COMPONENT_THIS_FRAME(HUD_VEHICLE_NAME);
	HIDE_HUD_COMPONENT_THIS_FRAME(HUD_DISTRICT_NAME);
	HIDE_HUD_COMPONENT_THIS_FRAME(HUD_STREET_NAME);

	DRAW_SCALEFORM_MOVIE_FULLSCREEN(InstructionalButtonsScaleformIndex, 0, 0, 0, 186, 0);	//HUD_COLOUR_INGAME_BG
	return;
}

void AddScaleformInstructionalButton(int iButtonSlotControl, int iButtonSlotInput, char* sText, bool reset)
{
	if (reset)
	{ ScaleformInstructionalButtons tmp; DisplayStruct = tmp; }	//Reset struct state

	if (DisplayStruct.ButtonCount >= 12)
		return;

	int i = DisplayStruct.ButtonCount;
	DisplayStruct.Buttons[i].iButtonSlotControl = iButtonSlotControl;
	DisplayStruct.Buttons[i].iButtonSlotInput = iButtonSlotInput;
	DisplayStruct.Buttons[i].sText = sText;
	DisplayStruct.ButtonCount++;
	return;
}
#pragma endregion

#pragma region Misc
bool SetAnimSpeed(Entity entity, const char* animDict, const char* animName, float speedMultiplier)
{
	if (IS_ENTITY_PLAYING_ANIM(entity, animDict, animName, 3))
	{
		SET_ENTITY_ANIM_SPEED(entity, animDict, animName, speedMultiplier);
		return true;
	}

	return false;
}

void PlayAmbientSpeech(Ped ped, char* speechName)
{
	AUDIO::SET_AUDIO_FLAG("IsDirectorModeActive", true);
	AUDIO::PLAY_PED_AMBIENT_SPEECH_NATIVE(ped, speechName, "SPEECH_PARAMS_FORCE_NORMAL", false);
	AUDIO::SET_AUDIO_FLAG("IsDirectorModeActive", false);
	return;
}
#pragma endregion