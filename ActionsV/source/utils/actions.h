#pragma once

bool NoSequenceIsActive();
void StopActiveSequence();
void UpdateSequences();

namespace SmokingSequence {
int GetForcedSequenceState();
bool IsSequenceActive();
void StartSequence();
void SetSequenceState(int state, bool shouldForceState = false);
void UpdateSequence();
}

namespace DrinkingSequence {
int GetForcedSequenceState();
bool IsSequenceActive();
void StartSequence();
void SetSequenceState(int state, bool shouldForceState = false);
void UpdateSequence();
}