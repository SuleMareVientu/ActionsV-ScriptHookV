#pragma once

void UpdateSequences();

namespace SmokingSequence {
bool IsSequenceActive();
void UpdateSequence();
}

namespace DrinkingSequence {
bool IsSequenceActive();
void UpdateSequence();
}