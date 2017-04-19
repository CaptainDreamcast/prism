#pragma once

// TODO: Refactor

#include <stdio.h>

#include "int.h"

#define CheckTextArraySize 64

uint8 SearchForString(FILE* InputFile, char StringThatIsSearched[], uint32* InputOffset, uint32 StartOffset, uint32 EndOffset);
void ReadArrayFromInputFile(FILE* InputFile, uint32 CurrentOffset, char DestinationArray[], uint8 DesiredArrayLength);
long ReadNumberFromInputFile(FILE* InputFile, uint32 CurrentOffset);
uint8 CompareStrings(char FirstArray[], char SecondArray[]);
uint8 ReadButtonsFromInputFile(FILE* InputFile, uint32 CurrentOffset);

uint8 CheckIfCopyingIsNecessary(char TemporaryAnimationName[], uint8* FrameID, uint8* AnimationAmount);
