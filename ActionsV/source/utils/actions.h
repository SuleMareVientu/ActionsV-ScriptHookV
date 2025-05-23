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

// Sound should always be played from playerPed and never Objects (sound stops in interiors)

static constexpr char* genericMessage = "Hold to stop";

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
	void Stop() { shouldStopSequence = true; return; }
	bool ShouldStop() const { return shouldStopSequence; }
	virtual void Update() = 0;

protected:
	int sequenceState = NULL;
	int nextSequenceState = NULL;
	bool isSequenceActive = false;
	bool shouldStopSequence = false;

	char* lastAnimDict = NULL;
	char* lastAnim = NULL;

	bool disabledControlsLastFrame = false;
	bool shouldPlayerStandStill = false;

	bool disableFirstPersonView = false;
	bool canSprint = false;
	float maxMoveBlendRatio = PEDMOVEBLENDRATIO_WALK;

	Timer stopTimer;
	int maxStopTimer = 10000;

	char* instructionalButtonsText = NULL;
	eControlType control = PLAYER_CONTROL;
	eControlAction input = INPUT_JUMP;
	Timer controlTimer;
	short holdTime = 250;

	void PlayAnimAndWait(char *animDict, char *anim, int flag, int nextState, float startPhase = 0.0f, float blendInSpeed = SLOW_BLEND_IN, float blendOutSpeed = SLOW_BLEND_OUT, bool standStill = false, int duration = -1);
	void PlayScriptedAnimAndWait(
		const int nextState,
		char* animDict = "",
		char* anim = "",
		const float phase = 0.0f,
		const float rate = 1.0f,
		const float weight = 1.0f,
		const int type = APT_EMPTY,
		const int filter = 0,
		const float blendInDelta = NORMAL_BLEND_DURATION,
		const float blendOutDelta = NORMAL_BLEND_DURATION,
		const int timeToPlay = -1,
		const int flags = AF_DEFAULT,
		const int ikFlags = AIK_NONE,
		const bool standStill = false);
	void SetPlayerControls();
	void SetPedMovementAndReactions() const;
};

class cSmokingSequence : public cSequence
{
public:
	void Update();

	cSmokingSequence() { maxStopTimer = 20000; instructionalButtonsText = "Smoke (hold to stop)"; }

private:
	enum SequenceState
	{
		FINISHED = SEQUENCE_FINISHED,
		STREAM_ASSETS_IN = SEQUENCE_STREAM_ASSETS_IN,
		INITIALIZED = SEQUENCE_INITIALIZED,
		WAITING_FOR_ANIMATION_TO_END = SEQUENCE_WAITING_FOR_ANIMATION_TO_END,
		FLUSH_ASSETS = SEQUENCE_FLUSH_ASSETS,
		ENTER_SMOKE,
		SMOKE,
		EXITING
	};

	Object item = NULL;

	void StopAllAnims();

	void StopAllPTFX();

	int cigarettePTFXHandle = NULL;
	int hasExhaledNose = NULL;
	void PlayPTFX();

	bool releasedArmAnim = false;
	void PlaySequence();

	bool GetAnimHold(char **animDict, char **anim);

	void SetState(int state);

	void UpdateControls();

	void ForceStop();
};

class cDrinkingSequence : public cSequence
{
public:
	void Update();

	cDrinkingSequence() { maxStopTimer = 5000; instructionalButtonsText = "Drink (hold to stop)"; }

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

class cLeafBlowerSequence : public cSequence
{
public:
	void Update();

	cLeafBlowerSequence() {
		maxStopTimer = 2000; canSprint = true; maxMoveBlendRatio = PEDMOVEBLENDRATIO_SPRINT;
		disableFirstPersonView = true;  instructionalButtonsText = "Toggle (hold to stop)";
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

	bool isUsingLeafBlower = false;
	Object item = NULL;

	void StopAllPTFXAndSounds();

	int leafBlowerPTFXHandle = NULL;
	int soundID = NULL;
	void PlayPTFXAndSound();

	void PlaySequence();

	void SetState(int state);

	void UpdateControls();

	void ForceStop();
};

class cJogSequence : public cSequence
{
public:
	void Update();

	cJogSequence() { maxStopTimer = 2000; instructionalButtonsText = genericMessage; disableFirstPersonView = true; }

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

	void PlaySequence();

	void SetState(int state);

	void UpdateControls();

	void ForceStop();
};

class cClipboardSequence : public cSequence
{
public:
	void Update();

	cClipboardSequence() {
		maxStopTimer = 2000; canSprint = true; maxMoveBlendRatio = PEDMOVEBLENDRATIO_SPRINT;
		disableFirstPersonView = true; instructionalButtonsText = genericMessage;
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

	void PlaySequence();

	void SetState(int state);

	void UpdateControls();

	void ForceStop();
};

class cGuitarSequence : public cSequence
{
public:
	void Update();

	cGuitarSequence() { maxStopTimer = 3500; instructionalButtonsText = genericMessage; }

private:
	enum SequenceState
	{
		FINISHED = SEQUENCE_FINISHED,
		STREAM_ASSETS_IN = SEQUENCE_STREAM_ASSETS_IN,
		INITIALIZED = SEQUENCE_INITIALIZED,
		WAITING_FOR_ANIMATION_TO_END = SEQUENCE_WAITING_FOR_ANIMATION_TO_END,
		FLUSH_ASSETS = SEQUENCE_FLUSH_ASSETS,
		LOOP,
		EXITING,
	};

	Object item = NULL;

	void StopAllAnims();

	void PlaySequence();

	void SetState(int state);

	void UpdateControls();

	void ForceStop();
};

class cBongosSequence : public cSequence
{
public:
	void Update();

	cBongosSequence() { maxStopTimer = 3500; instructionalButtonsText = genericMessage; }

private:
	enum SequenceState
	{
		FINISHED = SEQUENCE_FINISHED,
		STREAM_ASSETS_IN = SEQUENCE_STREAM_ASSETS_IN,
		INITIALIZED = SEQUENCE_INITIALIZED,
		WAITING_FOR_ANIMATION_TO_END = SEQUENCE_WAITING_FOR_ANIMATION_TO_END,
		FLUSH_ASSETS = SEQUENCE_FLUSH_ASSETS,
		LOOP,
		EXITING,
	};

	Object item = NULL;

	void StopAllAnims();

	void PlaySequence();

	void SetState(int state);

	void UpdateControls();

	void ForceStop();
};

class cMopSequence : public cSequence
{
public:
	void Update();

	cMopSequence() {
		maxStopTimer = 2000; canSprint = true; maxMoveBlendRatio = PEDMOVEBLENDRATIO_SPRINT;
		disableFirstPersonView = true; instructionalButtonsText = genericMessage;
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

	void PlaySequence();

	void SetState(int state);

	void UpdateControls();

	void ForceStop();
};

class cMopWithBucketSequence : public cSequence
{
public:
	void Update();

	cMopWithBucketSequence() { maxStopTimer = 2000; instructionalButtonsText = genericMessage; disableFirstPersonView = true; }

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

	Object mop = NULL;
	Object bucket = NULL;

	void PlaySequence();

	void SetState(int state);

	void UpdateControls();

	void ForceStop();
};

class cCameraSequence : public cSequence
{
public:
	void Update();

	cCameraSequence() { maxStopTimer = 2000; canSprint = true; maxMoveBlendRatio = PEDMOVEBLENDRATIO_SPRINT;
						disableFirstPersonView = true; instructionalButtonsText = "Take a picture (hold to stop)"; }

private:
	enum SequenceState
	{
		FINISHED = SEQUENCE_FINISHED,
		STREAM_ASSETS_IN = SEQUENCE_STREAM_ASSETS_IN,
		INITIALIZED = SEQUENCE_INITIALIZED,
		WAITING_FOR_ANIMATION_TO_END = SEQUENCE_WAITING_FOR_ANIMATION_TO_END,
		FLUSH_ASSETS = SEQUENCE_FLUSH_ASSETS,
		LOOP,
		TAKE_PHOTO,
		EXITING
	};

	Object camera = NULL;
	Object flashUnit = NULL;
	bool playFlashSound = false;

	void PlayPTFXAndSound();

	void PlaySequence();

	void SetState(int state);

	void UpdateControls();

	void ForceStop();
};

class cMobileTextSequence : public cSequence
{
public:
	void Update();

	cMobileTextSequence() { maxStopTimer = 9000; instructionalButtonsText = genericMessage; }

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

	void PlaySequence();

	void SetState(int state);

	void UpdateControls();

	void ForceStop();
};

class cShineTorchSequence : public cSequence
{
public:
	void Update();

	cShineTorchSequence() { maxStopTimer = 6000; instructionalButtonsText = genericMessage; }

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

	void PlaySequence();

	void SetState(int state);

	void UpdateControls();

	void ForceStop();
};

class cLiftCurlBarSequence : public cSequence
{
public:
	void Update();

	cLiftCurlBarSequence() { maxStopTimer = 10000; instructionalButtonsText = "Lift (hold to stop)"; }

private:
	enum SequenceState
	{
		FINISHED = SEQUENCE_FINISHED,
		STREAM_ASSETS_IN = SEQUENCE_STREAM_ASSETS_IN,
		INITIALIZED = SEQUENCE_INITIALIZED,
		WAITING_FOR_ANIMATION_TO_END = SEQUENCE_WAITING_FOR_ANIMATION_TO_END,
		FLUSH_ASSETS = SEQUENCE_FLUSH_ASSETS,
		LIFT,
		HOLD,
		EXITING,
	};

	Object item = NULL;

	void StopAllAnims();

	void PlaySequence();

	void SetState(int state);

	void UpdateControls();

	void ForceStop();
};

class cBinocularsSequence : public cSequence
{
public:
	void Update();

	cBinocularsSequence() { maxStopTimer = 6000; instructionalButtonsText = genericMessage; }

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

	void PlaySequence();

	void SetState(int state);

	void UpdateControls();

	void ForceStop();
};

class cHoldBumSignSequence : public cSequence
{
public:
	void Update();

	cHoldBumSignSequence() { maxStopTimer = 3500; instructionalButtonsText = genericMessage; }

private:
	enum SequenceState
	{
		FINISHED = SEQUENCE_FINISHED,
		STREAM_ASSETS_IN = SEQUENCE_STREAM_ASSETS_IN,
		INITIALIZED = SEQUENCE_INITIALIZED,
		WAITING_FOR_ANIMATION_TO_END = SEQUENCE_WAITING_FOR_ANIMATION_TO_END,
		FLUSH_ASSETS = SEQUENCE_FLUSH_ASSETS,
		LOOP,
		EXITING,
	};

	Object item = NULL;
	int itemHash = NULL;

	int GetBumSignPropHash() const;

	void StopAllAnims();

	void PlaySequence();

	void SetState(int state);

	void UpdateControls();

	void ForceStop();
};

class cFishingSequence : public cSequence
{
public:
	void Update();

	cFishingSequence() { maxStopTimer = 3500; instructionalButtonsText = genericMessage; }

private:
	enum SequenceState
	{
		FINISHED = SEQUENCE_FINISHED,
		STREAM_ASSETS_IN = SEQUENCE_STREAM_ASSETS_IN,
		INITIALIZED = SEQUENCE_INITIALIZED,
		WAITING_FOR_ANIMATION_TO_END = SEQUENCE_WAITING_FOR_ANIMATION_TO_END,
		FLUSH_ASSETS = SEQUENCE_FLUSH_ASSETS,
		LOOP,
		EXITING,
	};

	Object item = NULL;

	void StopAllAnims();

	void PlaySequence();

	void SetState(int state);

	void UpdateControls();

	void ForceStop();
};

void UpdateSequences();

// Sequences
extern cSmokingSequence smokingSequence;
extern cDrinkingSequence drinkingSequence;
extern cLeafBlowerSequence leafBlowerSequence;
extern cJogSequence jogSequence;
extern cClipboardSequence clipboardSequence;
extern cGuitarSequence guitarSequence;
extern cBongosSequence bongosSequence;
extern cMopSequence mopSequence;
extern cMopWithBucketSequence mopWithBucketSequence;
extern cCameraSequence cameraSequence;
extern cMobileTextSequence mobileTextSequence;
extern cShineTorchSequence shineTorchSequence;
extern cLiftCurlBarSequence liftCurlBarSequence;
extern cBinocularsSequence binocularsSequence;
extern cHoldBumSignSequence holdBumSignSequence;
extern cFishingSequence fishingSequence;