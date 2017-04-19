#pragma once

// TODO: REFACTOR

#include "../common/int.h"

void CopyOverTextureFunction(char TemporaryAnimationName[], char FinalOutputFileDir[], char TextureInputFolder[], uint8 FrameID, uint8 FrameAmount, char ConverterDirectory[], char CompressorDirectory[]);
void CopyOverSoundEffectFunction(char TemporaryAnimationName[], char FinalOutputFileDir[], char TextureInputFolder[], uint8 FrameID);

uint8 CheckIfCopyingSoundEffectIsNecessary(char TemporaryAnimationName[], uint8* FrameID, uint8* AnimationAmount);
