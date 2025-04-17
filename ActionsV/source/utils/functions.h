#pragma once
#include <natives.h>
#include "..\globals.h"
#include <string>

class Timer {
	int gameTimer = 0;
	int offset = 0;
public:
	void Set(const int value) { gameTimer = GET_GAME_TIMER(); offset = value; return; }
	void Reset() { gameTimer = GET_GAME_TIMER(); offset = 0; return; }
	int Get() const { return (GET_GAME_TIMER() - gameTimer + offset); }
	Timer(const int startVal = 0) { offset = startVal; }
};

inline Player GetPlayer() { return PLAYER_ID(); }
inline Ped GetPlayerPed() { return PLAYER_PED_ID(); }
inline Vector3 GetPlayerCoords() { return GET_ENTITY_COORDS(PLAYER_PED_ID(), false); }

void EnablePedConfigFlag(Ped ped, int flag);
void DisablePedConfigFlag(Ped ped, int flag);
void EnablePedResetFlag(Ped ped, int flag);
void DisablePedResetFlag(Ped ped, int flag);

void Print(char* string, int ms = 1);
void PrintInt(int value, int ms = 1);
void PrintFloat(float value, int ms = 1);
void PrintHelp(char* string, bool playSound = false, int overrideDuration = -1);

bool RequestModel(Hash model);
Object CreateObject(Hash model, float locX = NULL, float locY = NULL, float locZ = NULL, float rotX = NULL, float rotY = NULL, float rotZ = NULL);
void DeleteEntity(Entity* obj);
bool RequestAnimDict(char* animDict);
bool StopAnimTask(Entity entity, const char* animDict, const char* animName, float blendDelta = REALLY_SLOW_BLEND_OUT);
bool RequestClipSet(char* animSet);
bool RequestAudioBank(char* bank);
bool StopAudioStream();
bool RequestPTFXAsset(char* asset);

bool StartParticleFxNonLoopedOnEntity(const char* PTFXAsset, const char* effectName, Entity entity, float scale = 1.0f, float offsetX = 0.0f, float offsetY = 0.0f, float offsetZ = 0.0f, float rotX = 0.0f, float rotY = 0.0f, float rotZ = 0.0f, bool axisX = false, bool axisY = false, bool axisZ = false);
int StartParticleFxLoopedOnPedBone(const char* PTFXAsset, const char* effectName, Ped ped, int boneIndex, float scale = 1.0f, float xOffset = 0.0f, float yOffset = 0.0f, float zOffset = 0.0f, float xRot = 0.0f, float yRot = 0.0f, float zRot = 0.0f, bool xAxis = false, bool yAxis = false, bool zAxis = false);
bool StartParticleFxNonLoopedOnPedBone(const char* PTFXAsset, const char* effectName, Ped ped, int boneIndex, float scale = 1.0f, float offsetX = 0.0f, float offsetY = 0.0f, float offsetZ = 0.0f, float rotX = 0.0f, float rotY = 0.0f, float rotZ = 0.0f, bool axisX = false, bool axisY = false, bool axisZ = false);
void StopPTFX(int* PTFXHandle);

bool AdditionalChecks(Ped ped, bool countEnemies = false);
void DisablePlayerActionsThisFrame(bool canSprint = false, float maxMoveBlendRatio = PEDMOVEBLENDRATIO_WALK);
void DisablePlayerControlThisFrame();

void RunScaleformInstructionalButtons(bool refresh = true);
void AddScaleformInstructionalButton(int iButtonSlotControl, int iButtonSlotInput, char* sText, bool reset = false);

bool SetAnimSpeed(Entity entity, const char* animDict, const char* animName, float speedMultiplier);
void PlayAmbientSpeech(Ped ped, char* speechName);
int GetRandomIntInRange(int startRange = 0, int endRange = 65535);
void PlayScriptedAnim(
	const Ped ped,
	const char* dictionary0 = "",
	const char* anim0 = "",
	const float phase0 = 0.0f,
	const float rate0 = 1.0f,
	const float weight0 = 1.0f,

	const int type = APT_EMPTY,
	const int filter = 0,
	const float blendInDelta = NORMAL_BLEND_DURATION,
	const float blendOutDelta = NORMAL_BLEND_DURATION,
	const int timeToPlay = -1,
	const int flags = AF_DEFAULT,
	const int ikFlags = AIK_NONE);
void SplitString(const char* charStr, std::string arr[], const int arrSize);
int GetPadControlFromString(const std::string &str);
int GetVKFromString(const std::string &str);

inline void PlayAnimTask(Ped ped, const char* animDictionary, const char* animationName, int flag = AF_DEFAULT, float startPhase = 0.0f, float blendInSpeed = SLOW_BLEND_IN, float blendOutSpeed = SLOW_BLEND_OUT, int duration = -1)
{ TASK_PLAY_ANIM(ped, animDictionary, animationName, blendInSpeed, blendOutSpeed, duration, flag, startPhase, false, false, false); }