#pragma once
#include "functions.h"
#include "../globals.h"

#define SEQUENCE_FINISHED 0
#define SEQUENCE_STREAM_ASSETS_IN 1
#define SEQUENCE_INITIALIZED 2
#define SEQUENCE_WAITING_FOR_ANIMATION_TO_END 3
#define SEQUENCE_FLUSH_ASSETS 4

/*	All sequence enums must follow this structure
enum eSequenceState
{
	FINISHED = SEQUENCE_FINISHED,
	STREAM_ASSETS_IN = SEQUENCE_STREAM_ASSETS_IN,
	INITIALIZED = SEQUENCE_INITIALIZED,
	WAITING_FOR_ANIMATION_TO_END = SEQUENCE_WAITING_FOR_ANIMATION_TO_END,
	FLUSH_ASSETS = SEQUENCE_FLUSH_ASSETS,
	...
};
*/

class cSequence
{
public:
	bool IsActive() const
	{
		if (sequenceState == SEQUENCE_FINISHED)
			return false;

		return true;
	}

	void Start();
	int GetState() const { return sequenceState; }
	void Stop() { shouldStopSequence = true; return; };
	bool ShouldStop() const { return shouldStopSequence; };

protected:
	int sequenceState = NULL;
	int nextSequenceState = NULL;
	bool isSequenceActive = false;
	bool shouldStopSequence = false;
	char* lastAnimDict = NULL;
	char* lastAnim = NULL;
	bool disabledControlsLastFrame = false;
	bool shouldPlayerStandStill = false;
	Timer stopTimer;
	int maxStopTimer = 10000;
	ControlType control = PLAYER_CONTROL;
	ControlAction input = INPUT_JUMP;
	Timer controlTimer;
	short holdTime = 250;

	void PlayAnim(char *animDict, char *anim, int flag, int duration = -1);
	void PlayAnimAndWait(char *animDict, char *anim, int flag, int nextState, float startPhase = 0.0f, float blendInSpeed = 1.5f, float blendOutSpeed = -1.5f, bool standStill = false, int duration = -1);
	void SetPlayerControls();
	void SetPedMovementAndReactions() const;
};

constexpr int cigaretteHash = 0x783A4BE3;		//Prop_AMB_Ciggy_01
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
class cSmokingSequence : public cSequence
{
public:
	void Update();

	cSmokingSequence() {
		maxStopTimer = 20000;
	}

private:
	enum SequenceState
	{
		FINISHED = SEQUENCE_FINISHED,
		STREAM_ASSETS_IN = SEQUENCE_STREAM_ASSETS_IN,
		INITIALIZED = SEQUENCE_INITIALIZED,
		WAITING_FOR_ANIMATION_TO_END = SEQUENCE_WAITING_FOR_ANIMATION_TO_END,
		FLUSH_ASSETS = SEQUENCE_FLUSH_ASSETS,
		LOOP,
		EXITING
	};

	Object item = NULL;

	void StopAllAnims();

	void StopPTFX(int *PTFXHandle);

	void StopAllPTFX();

	int cigarettePTFXHandle = NULL;
	int hasExhaledNose = NULL;
	void PlayPTFX();

	void PlaySequence();

	bool GetAnimHold(char **animDict, char **anim);

	void SetState(int state);

	void UpdateControls();

	void ForceStop();
};

constexpr int beerHash = 0x28BE7556;		//Prop_AMB_Beer_Bottle
constexpr char* drinkingAnimDict = "mp_player_intdrink";
constexpr char* drinkingEnterAnim = "intro_bottle";
constexpr char* drinkingBaseAnim = "loop_bottle";
constexpr char* drinkingExitAnim = "outro_bottle";
class cDrinkingSequence : public cSequence
{
public:
	void Update();

	cDrinkingSequence() {
		maxStopTimer = 5000;
	}

private:
	enum SequenceState
	{
		FINISHED = SEQUENCE_FINISHED,
		STREAM_ASSETS_IN = SEQUENCE_STREAM_ASSETS_IN,
		INITIALIZED = SEQUENCE_INITIALIZED,
		WAITING_FOR_ANIMATION_TO_END = SEQUENCE_WAITING_FOR_ANIMATION_TO_END,
		FLUSH_ASSETS = SEQUENCE_FLUSH_ASSETS,
		ENTER_DRINK,
		DRINK,
		HOLD,
		EXITING,
	};

	Object item = NULL;

	void StopAllAnims();

	void PlaySequence();

	bool GetAnimHold(char **animDict, char **anim);

	void SetState(int state);

	void UpdateControls();

	void ForceStop();
};

void UpdateSequences();

// Sequences
extern cSmokingSequence smokingSequence;
extern cDrinkingSequence drinkingSequence;
