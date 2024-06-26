#pragma once

class Timer {
	int gameTimer = 0;
public:
	void Set(int value);
	int Get();
};

void EnablePedConfigFlag(Ped ped, int flag);
void DisablePedConfigFlag(Ped ped, int flag);
void EnablePedResetFlag(Ped ped, int flag);
void DisablePedResetFlag(Ped ped, int flag);
void Print(char* string, int ms = 1);
void PrintInt(int value, int ms = 1);
void PrintFloat(float value, int ms = 1);
void PrintHelp(char* string, bool playSound = false, int overrideDuration = -1);
bool RequestAnimDict(char* animDict);
bool RequestModel(Hash model);
Object CreateObject(Hash model, float locX = NULL, float locY = NULL, float locZ = NULL, float rotX = NULL, float rotY = NULL, float rotZ = NULL);
void DeleteObject(Object* obj);
bool AdditionalChecks(Ped ped, bool countEnemies = false);
void DisablePlayerActionsThisFrame();
void DisablePlayerControlThisFrame();
void PlayAmbientSpeech(Ped ped, char* speechName);
void RunScaleformInstructionalButtons(bool refresh = true);
void AddScaleformInstructionalButton(int iButtonSlotControl, int iButtonSlotInput, char* sText, bool reset = false);