#pragma once

void UpdateSequences();

namespace SmokingSequence {
bool IsSequenceActive();
void StartSequence();
void SetSequenceState(int state, bool shouldForceState = false);
void UpdateSequence();
}

namespace DrinkingSequence {
bool IsSequenceActive();
void StartSequence();
void SetSequenceState(int state, bool shouldForceState = false);
void UpdateSequence();
}