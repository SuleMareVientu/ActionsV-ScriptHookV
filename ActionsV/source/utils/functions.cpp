//ScriptHook
#include <natives.h>
// #include <types.h> //Already included in globals.h
//Custom
#include "functions.h"
#include "..\script.h"
#include <unordered_map>
#include <sstream>
#include <algorithm>

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
	std::string tmpStr = str;
	std::transform(tmpStr.begin(), tmpStr.end(), tmpStr.begin(), ::toupper);
	auto it = mapPadControls.find(tmpStr);
	if (it != mapPadControls.end())
		return it->second;
	else
		return -1;
}

static const std::unordered_map<std::string, int> mapVKs = {
	{"LBUTTON", 0X01}, {"RBUTTON", 0X02}, {"CANCEL", 0X03}, {"MBUTTON", 0X04}, {"XBUTTON1", 0X05},
	{"XBUTTON2", 0X06}, {"BACK", 0X08}, {"TAB", 0X09}, {"CLEAR", 0X0C}, {"RETURN", 0X0D}, {"SHIFT", 0X10},
	{"CONTROL", 0X11}, {"MENU", 0X12}, {"PAUSE", 0X13}, {"CAPITAL", 0X14}, {"HANGUL", 0X15}, {"JUNJA", 0X17},
	{"FINAL", 0X18}, {"HANJA", 0X19}, {"ESCAPE", 0X1B}, {"CONVERT", 0X1C}, {"NONCONVERT", 0X1D}, {"ACCEPT", 0X1E},
	{"MODECHANGE", 0X1F}, {"SPACE", 0X20}, {"PRIOR", 0X21}, {"NEXT", 0X22}, {"END", 0X23}, {"HOME", 0X24},
	{"LEFT", 0X25}, {"UP", 0X26}, {"RIGHT", 0X27}, {"DOWN", 0X28}, {"SELECT", 0X29}, {"PRINT", 0X2A},
	{"EXECUTE", 0X2B}, {"SNAPSHOT", 0X2C}, {"INSERT", 0X2D}, {"DELETE", 0X2E}, {"HELP", 0X2F}, {"0", 0X30},
	{"1", 0X31}, {"2", 0X32}, {"3", 0X33}, {"4", 0X34}, {"5", 0X35}, {"6", 0X36},
	{"7", 0X37}, {"8", 0X38}, {"9", 0X39}, {"A", 0X41}, {"B", 0X42}, {"C", 0X43},
	{"D", 0X44}, {"E", 0X45}, {"F", 0X46}, {"G", 0X47}, {"H", 0X48}, {"I", 0X49},
	{"J", 0X4A}, {"K", 0X4B}, {"L", 0X4C}, {"M", 0X4D}, {"N", 0X4E}, {"O", 0X4F},
	{"P", 0X50}, {"Q", 0X51}, {"R", 0X52}, {"S", 0X53}, {"T", 0X54}, {"U", 0X55},
	{"V", 0X56}, {"W", 0X57}, {"X", 0X58}, {"Y", 0X59}, {"Z", 0X5A}, {"LWIN", 0X5B},
	{"RWIN", 0X5C}, {"APPS", 0X5D}, {"SLEEP", 0X5F}, {"NUMPAD0", 0X60}, {"NUMPAD1", 0X61}, {"NUMPAD2", 0X62},
	{"NUMPAD3", 0X63}, {"NUMPAD4", 0X64}, {"NUMPAD5", 0X65}, {"NUMPAD6", 0X66}, {"NUMPAD7", 0X67}, {"NUMPAD8", 0X68},
	{"NUMPAD9", 0X69}, {"MULTIPLY", 0X6A}, {"ADD", 0X6B}, {"SEPARATOR", 0X6C}, {"SUBTRACT", 0X6D}, {"DECIMAL", 0X6E},
	{"DIVIDE", 0X6F}, {"F1", 0X70}, {"F2", 0X71}, {"F3", 0X72}, {"F4", 0X73}, {"F5", 0X74},
	{"F6", 0X75}, {"F7", 0X76}, {"F8", 0X77}, {"F9", 0X78}, {"F10", 0X79}, {"F11", 0X7A},
	{"F12", 0X7B}, {"F13", 0X7C}, {"F14", 0X7D}, {"F15", 0X7E}, {"F16", 0X7F}, {"F17", 0X80},
	{"F18", 0X81}, {"F19", 0X82}, {"F20", 0X83}, {"F21", 0X84}, {"F22", 0X85}, {"F23", 0X86},
	{"F24", 0X87}, {"NAVIGATIONVIEW", 0X88}, {"NAVIGATIONMENU", 0X89}, {"NAVIGATIONUP", 0X8A}, {"NAVIGATIONDOWN", 0X8B},
	{"NAVIGATIONLEFT", 0X8C}, {"NAVIGATIONRIGHT", 0X8D}, {"NAVIGATIONACCEPT", 0X8E}, {"NAVIGATIONCANCEL", 0X8F},
	{"NUMLOCK", 0X90}, {"SCROLL", 0X91}, {"LSHIFT", 0XA0}, {"RSHIFT", 0XA1}, {"LCONTROL", 0XA2}, {"RCONTROL", 0XA3},
	{"LMENU", 0XA4}, {"RMENU", 0XA5}, {"BROWSER_BACK", 0XA6}, {"BROWSER_FORWARD", 0XA7}, {"BROWSER_REFRESH", 0XA8},
	{"BROWSER_STOP", 0XA9}, {"BROWSER_SEARCH", 0XAA}, {"BROWSER_FAVORITES", 0XAB},
	{"BROWSER_HOME", 0XAC}, {"VOLUME_MUTE", 0XAD}, {"VOLUME_DOWN", 0XAE}, {"VOLUME_UP", 0XAF},
	{"MEDIA_NEXT_TRACK", 0XB0}, {"MEDIA_PREV_TRACK", 0XB1}, {"MEDIA_STOP", 0XB2}, {"MEDIA_PLAY_PAUSE", 0XB3},
	{"LAUNCH_MAIL", 0XB4}, {"LAUNCH_MEDIA_SELECT", 0XB5}, {"LAUNCH_APP1", 0XB6}, {"LAUNCH_APP2", 0XB7},
	{"OEM_1", 0XBA}, {"OEM_PLUS", 0XBB}, {"OEM_COMMA", 0XBC}, {"OEM_MINUS", 0XBD}, {"OEM_PERIOD", 0XBE},
	{"OEM_2", 0XBF}, {"OEM_3", 0XC0}, {"GAMEPAD_A", 0XC3}, {"GAMEPAD_B", 0XC4}, {"GAMEPAD_X", 0XC5}, {"GAMEPAD_Y", 0XC6},
	{"GAMEPADRIGHTBUMPER", 0XC7}, {"GAMEPADLEFTBUMPER", 0XC8}, {"GAMEPADLEFTTRIGGER", 0XC9}, {"GAMEPADRIGHTTRIGGER", 0XCA},
	{"GAMEPADDPADUP", 0XCB}, {"GAMEPADDPADDOWN", 0XCC}, {"GAMEPADDPADLEFT", 0XCD}, {"GAMEPADDPADRIGHT", 0XCE},
	{"GAMEPADMENU", 0XCF}, {"GAMEPADVIEW", 0XD0}, {"GAMEPADLEFTSTICKBTN", 0XD1}, {"GAMEPADRIGHTSTICKBTN", 0XD2},
	{"GAMEPADLEFTSTICKUP", 0XD3}, {"GAMEPADLEFTSTICKDOWN", 0XD4}, {"GAMEPADLEFTSTICKRIGHT", 0XD5}, {"GAMEPADLEFTSTICKLEFT", 0XD6},
	{"GAMEPADRIGHTSTICKUP", 0XD7}, {"GAMEPADRIGHTSTICKDOWN", 0XD8}, {"GAMEPADRIGHTSTICKRIGHT", 0XD9}, {"GAMEPADRIGHTSTICKLEFT", 0XDA},
	{"OEM_4", 0XDB}, {"OEM_5", 0XDC}, {"OEM_6", 0XDD}, {"OEM_7", 0XDE}, {"OEM_8", 0XDF},	 {"OEM_102", 0XE2},
	{"PROCESSKEY", 0XE5}, {"PACKET", 0XE7}, {"ATTN", 0XF6}, {"CRSEL", 0XF7}, {"EXSEL", 0XF8}, {"EREOF", 0XF9},
	{"PLAY", 0XFA}, {"ZOOM", 0XFB}, {"NONAME", 0XFC}, {"PA1", 0XFD}, {"OEM_CLEAR", 0XFE}
};
int GetVKFromString(const std::string &str)
{
	std::string tmpStr = str;
	std::transform(tmpStr.begin(), tmpStr.end(), tmpStr.begin(), ::toupper);
	auto it = mapVKs.find(tmpStr);
	if (it != mapVKs.end())
		return it->second;
	else
		return -1;
}
#pragma endregion