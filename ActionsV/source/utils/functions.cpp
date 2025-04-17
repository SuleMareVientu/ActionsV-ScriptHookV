//ScriptHook
#include <natives.h>
// #include <types.h> //Already included in globals.h
//Custom
#include "functions.h"
#include "..\script.h"
#include <unordered_map>
#include <sstream>

#pragma region Ped Flags
void EnablePedConfigFlag(Ped ped, int flag)
{
	if (!GET_PED_CONFIG_FLAG(ped, flag, false))
		SET_PED_CONFIG_FLAG(ped, flag, true);
	return;
}

void DisablePedConfigFlag(Ped ped, int flag)
{
	if (GET_PED_CONFIG_FLAG(ped, flag, false))
		SET_PED_CONFIG_FLAG(ped, flag, false);
	return;
}

void EnablePedResetFlag(Ped ped, int flag)
{
	if (!GET_PED_RESET_FLAG(ped, flag))
		SET_PED_RESET_FLAG(ped, flag, true);
	return;
}

void DisablePedResetFlag(Ped ped, int flag)
{
	if (GET_PED_RESET_FLAG(ped, flag))
		SET_PED_RESET_FLAG(ped, flag, false);
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
	Object obj = CREATE_OBJECT_NO_OFFSET(model, locX, locY, locZ, false, true, false);
	if (rotX != NULL || rotY != NULL || rotZ != NULL)
		SET_ENTITY_ROTATION(obj, rotX, rotY, rotZ, 2, false);

	return obj;
}

void DeleteEntity(Entity* obj)
{
	if (DOES_ENTITY_EXIST(*obj))
	{
		if (IS_ENTITY_ATTACHED(*obj))
			DETACH_ENTITY(*obj, false, false);

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
bool showScaleformInstructionalButtons = true;
static ScaleformInstructionalButtons DisplayStruct;
static int InstructionalButtonsScaleformIndex = NULL;
void RunScaleformInstructionalButtons(bool refresh)
{
	if (!showScaleformInstructionalButtons)
		return;

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
AnimData nullAnimData;
AnimData animData;
void PlayScriptedAnim(const Ped ped, const char* dictionary0, const char* anim0, const float phase0, const float rate0, const float weight0, const int type, const int filter, const float blendInDelta, const float blendOutDelta, const int timeToPlay, const int flags, const int ikFlags)
{
	SetAnimData(animData,
		dictionary0, anim0, phase0, rate0, weight0,
		type, filter, blendInDelta, blendOutDelta, timeToPlay, flags, ikFlags);
	TASK_SCRIPTED_ANIMATION(
		ped,
		reinterpret_cast<int*>(&animData),
		reinterpret_cast<int*>(&nullAnimData),
		reinterpret_cast<int*>(&nullAnimData),
		blendInDelta,
		blendOutDelta
	);
	return;
}

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
	SET_AUDIO_FLAG("IsDirectorModeActive", true);
	PLAY_PED_AMBIENT_SPEECH_NATIVE(ped, speechName, "SPEECH_PARAMS_FORCE_NORMAL", false);
	SET_AUDIO_FLAG("IsDirectorModeActive", false);
	return;
}

int GetRandomIntInRange(int startRange, int endRange)
{
	endRange += 1;
	SET_RANDOM_SEED(GET_GAME_TIMER());
	return GET_RANDOM_INT_IN_RANGE(startRange, endRange);
}

void SplitString(const char* charStr, std::string arr[], const int arrSize)
{
	constexpr char space = 0x20; constexpr char tab = 0x09; constexpr char comma = 0x2C;
	std::string str; str = charStr;
	str.erase(std::remove(str.begin(), str.end(), space), str.end());
	str.erase(std::remove(str.begin(), str.end(), tab), str.end());
	std::istringstream ss(str);
	std::string token;

	int i = 0;
	while (std::getline(ss, token, comma) && i < arrSize)
	{
		arr[i] = token;
		i++;
	}
	return;
}

static const std::unordered_map<std::string, int> mapPadControls = {
	{"PAD_UP", INPUT_FRONTEND_UP},
	{"PAD_DOWN", INPUT_FRONTEND_DOWN},
	{"PAD_RIGHT", INPUT_FRONTEND_RIGHT},
	{"PAD_LEFT", INPUT_FRONTEND_LEFT},
	{"B_UP", INPUT_FRONTEND_Y},			// Y	
	{"B_DOWN", INPUT_FRONTEND_ACCEPT},	// A	
	{"B_RIGHT", INPUT_FRONTEND_CANCEL},	// B
	{"B_LEFT", INPUT_FRONTEND_X},		// X
	{"RT", INPUT_FRONTEND_RT},
	{"LT", INPUT_FRONTEND_LT},
	{"RB", INPUT_FRONTEND_RB},
	{"LB", INPUT_FRONTEND_LB},
	{"RSB", INPUT_FRONTEND_RS},
	{"LSB", INPUT_FRONTEND_LS},
	{"START", INPUT_FRONTEND_PAUSE},
	{"BACK", INPUT_FRONTEND_SELECT}
};
int GetPadControlFromString(const std::string &str)
{	
	auto it = mapPadControls.find(str);
	if (it != mapPadControls.end())
		return it->second;
	else
		return -1;
}

static const std::unordered_map<std::string, int> mapVKs = {
	{"LBUTTON", 0x01}, {"RBUTTON", 0x02}, {"CANCEL", 0x03}, {"MBUTTON", 0x04}, {"XBUTTON1", 0x05},
	{"XBUTTON2", 0x06}, {"BACK", 0x08}, {"TAB", 0x09}, {"CLEAR", 0x0C}, {"RETURN", 0X0D}, {"SHIFT", 0x10},
	{"CONTROL", 0x11}, {"MENU", 0x12}, {"PAUSE", 0x13}, {"CAPITAL", 0x14}, {"HANGUL", 0x15}, {"JUNJA", 0x17},
	{"FINAL", 0x18}, {"HANJA", 0x19}, {"ESCAPE", 0x1B}, {"CONVERT", 0x1C}, {"NONCONVERT", 0x1D}, {"ACCEPT", 0x1E},
	{"MODECHANGE", 0x1F}, {"SPACE", 0x20}, {"PRIOR", 0x21}, {"NEXT", 0x22}, {"END", 0x23}, {"HOME", 0x24},
	{"LEFT", 0x25}, {"UP", 0x26}, {"RIGHT", 0x27}, {"DOWN", 0x28}, {"SELECT", 0x29}, {"PRINT", 0x2A},
	{"EXECUTE", 0x2B}, {"SNAPSHOT", 0x2C}, {"INSERT", 0x2D}, {"DELETE", 0x2E}, {"HELP", 0x2F}, {"0", 0x30},
	{"1", 0x31}, {"2", 0x32}, {"3", 0x33}, {"4", 0x34}, {"5", 0x35}, {"6", 0x36},
	{"7", 0x37}, {"8", 0x38}, {"9", 0x39}, {"A", 0x41}, {"B", 0x42}, {"C", 0x43},
	{"D", 0x44}, {"E", 0x45}, {"F", 0x46}, {"G", 0x47}, {"H", 0x48}, {"I", 0x49},
	{"J", 0x4A}, {"K", 0x4B}, {"L", 0x4C}, {"M", 0x4D}, {"N", 0x4E}, {"O", 0x4F},
	{"P", 0x50}, {"Q", 0x51}, {"R", 0x52}, {"S", 0x53}, {"T", 0x54}, {"U", 0x55},
	{"V", 0x56}, {"W", 0x57}, {"X", 0x58}, {"Y", 0x59}, {"Z", 0x5A}, {"LWIN", 0x5B},
	{"RWIN", 0x5C}, {"APPS", 0x5D}, {"SLEEP", 0x5F}, {"NUMPAD0", 0x60}, {"NUMPAD1", 0x61}, {"NUMPAD2", 0x62},
	{"NUMPAD3", 0x63}, {"NUMPAD4", 0x64}, {"NUMPAD5", 0x65}, {"NUMPAD6", 0x66}, {"NUMPAD7", 0x67}, {"NUMPAD8", 0x68},
	{"NUMPAD9", 0x69}, {"MULTIPLY", 0x6A}, {"ADD", 0x6B}, {"SEPARATOR", 0x6C}, {"SUBTRACT", 0x6D}, {"DECIMAL", 0x6E},
	{"DIVIDE", 0x6F}, {"F1", 0x70}, {"F2", 0x71}, {"F3", 0x72}, {"F4", 0x73}, {"F5", 0x74},
	{"F6", 0x75}, {"F7", 0x76}, {"F8", 0x77}, {"F9", 0x78}, {"F10", 0x79}, {"F11", 0x7A},
	{"F12", 0x7B}, {"F13", 0x7C}, {"F14", 0x7D}, {"F15", 0x7E}, {"F16", 0x7F}, {"F17", 0x80},
	{"F18", 0x81}, {"F19", 0x82}, {"F20", 0x83}, {"F21", 0x84}, {"F22", 0x85}, {"F23", 0x86},
	{"F24", 0x87}, {"NavigationView", 0x88}, {"NavigationMenu", 0x89}, {"NavigationUp", 0x8A}, {"NavigationDown", 0x8B},
	{"NavigationLeft", 0x8C}, {"NavigationRight", 0x8D}, {"NavigationAccept", 0x8E}, {"NavigationCancel", 0x8F},
	{"NUMLOCK", 0x90}, {"SCROLL", 0x91}, {"LSHIFT", 0xA0}, {"RSHIFT", 0xA1}, {"LCONTROL", 0xA2}, {"RCONTROL", 0xA3},
	{"LMENU", 0xA4}, {"RMENU", 0xA5}, {"BROWSER_BACK", 0xA6}, {"BROWSER_FORWARD", 0xA7}, {"BROWSER_REFRESH", 0xA8},
	{"BROWSER_STOP", 0xA9}, {"BROWSER_SEARCH", 0xAA}, {"BROWSER_FAVORITES", 0xAB},
	{"BROWSER_HOME", 0xAC}, {"VOLUME_MUTE", 0xAD}, {"VOLUME_DOWN", 0xAE}, {"VOLUME_UP", 0xAF},
	{"MEDIA_NEXT_TRACK", 0xB0}, {"MEDIA_PREV_TRACK", 0xB1}, {"MEDIA_STOP", 0xB2}, {"MEDIA_PLAY_PAUSE", 0xB3},
	{"LAUNCH_MAIL", 0xB4}, {"LAUNCH_MEDIA_SELECT", 0xB5}, {"LAUNCH_APP1", 0xB6}, {"LAUNCH_APP2", 0xB7},
	{"OEM_1", 0xBA}, {"OEM_PLUS", 0xBB}, {"OEM_COMMA", 0xBC}, {"OEM_MINUS", 0xBD}, {"OEM_PERIOD", 0xBE},
	{"OEM_2", 0xBF}, {"OEM_3", 0xC0}, {"Gamepad_A", 0xC3}, {"Gamepad_B", 0xC4}, {"Gamepad_X", 0xC5}, {"Gamepad_Y", 0xC6},
	{"GamepadRightBumper", 0xC7}, {"GamepadLeftBumper", 0xC8}, {"GamepadLeftTrigger", 0xC9}, {"GamepadRightTrigger", 0xCA},
	{"GamepadDPadUp", 0xCB}, {"GamepadDPadDown", 0xCC}, {"GamepadDPadLeft", 0xCD}, {"GamepadDPadRight", 0xCE},
	{"GamepadMenu", 0xCF}, {"GamepadView", 0xD0}, {"GamepadLeftStickBtn", 0xD1}, {"GamepadRightStickBtn", 0xD2},
	{"GamepadLeftStickUp", 0xD3}, {"GamepadLeftStickDown", 0xD4}, {"GamepadLeftStickRight", 0xD5}, {"GamepadLeftStickLeft", 0xD6},
	{"GamepadRightStickUp", 0xD7}, {"GamepadRightStickDown", 0xD8}, {"GamepadRightStickRight", 0xD9}, {"GamepadRightStickLeft", 0xDA},
	{"OEM_4", 0xDB}, {"OEM_5", 0xDC}, {"OEM_6", 0xDD}, {"OEM_7", 0xDE}, {"OEM_8", 0xDF},	 {"OEM_102", 0xE2},
	{"PROCESSKEY", 0xE5}, {"PACKET", 0xE7}, {"ATTN", 0xF6}, {"CRSEL", 0xF7}, {"EXSEL", 0xF8}, {"EREOF", 0xF9},
	{"PLAY", 0xFA}, {"ZOOM", 0xFB}, {"NONAME", 0xFC}, {"PA1", 0xFD}, {"OEM_CLEAR", 0xFE}
};
int GetVKFromString(const std::string &str)
{
	auto it = mapVKs.find(str);
	if (it != mapVKs.end())
		return it->second;
	else
		return -1;
}
#pragma endregion