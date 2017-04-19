#pragma once

#include "common/header.h"

// TODO - REFACTOR THIS PART

#include <stdint.h>

#define MaximumStoryModeLength				30

#define StoryBoardMaximumTextSize			0x100
#define StoryBoardMaximumTextureAmount			5
#define StoryBoardMaximumTextAmount			3
#define StoryBoardMaximumSoundEffectAmount		2

#define StoryBoardDestroyTextureIdentifier		0
#define StoryBoardLoadTextureIdentifier			1
#define StoryBoardChangeTextureIdentifier		2

#define StoryBoardTextureStructIdentifier		0
#define StoryBoardTextStructIdentifier			1
#define StoryBoardSoundEffectStructIdentifier		2
#define StoryBoardStructAmount				3

#define StoryBoardDestroyTextIdentifier			0

#define StoryBoardDestroySoundEffectIdentifier	0

#pragma pack(push, 1)

typedef struct {
	uint16_t ActionAmount;
	uint8_t Separator1[14];

	uint8_t ReservedSpace[16];
} StoryBoardUberHeaderStruct; // TODO: refactor Dolmexica's naming conventions


typedef struct {

	uint8_t WhichAction;
	uint8_t WaitForButtonInput;
	uint8_t SoundTrack;
	uint8_t Separator1;

	uint16_t Duration;
	uint8_t Separator2[2];

	uint8_t TextureStructAmount;
	uint8_t TextStructAmount;
	uint8_t SoundEffectStructAmount;
	uint8_t Separator3;

	uint8_t Separator4[4]; //16

	uint8_t ReservedSpace[16];

} StoryBoardHeaderStruct;


typedef struct {

	uint8_t TextureID;
	uint8_t TextureAction;
	uint8_t TextureNameID;
	uint8_t Separator1;

	uint8_t WhichFrame;
	uint8_t FrameAmount;
	uint8_t Speed;
	uint8_t Loop;

	uint16_t PositionX;
	uint16_t PositionY;

	uint16_t SizeX;
	uint16_t SizeY; //16

	float TexturePositionX1;
	float TexturePositionY1;
	float TexturePositionX2;
	float TexturePositionY2; //32

	short MovementX;
	short MovementY;

	uint16_t PositionZ;
	uint8_t Separator2[2];

	int TextureSizeX;
	int TextureSizeY; //48

	uint8_t ReservedSpace[16];

} StoryBoardTextureStruct;

typedef struct {

	uint8_t TextID;
	uint8_t TextAction;
	uint8_t BuildUp;
	uint8_t BuildUpSpeed;

	uint16_t PositionX;
	uint16_t PositionY;

	uint16_t SizeX;
	uint16_t SizeY;

	uint16_t FontSizeX;
	uint16_t FontSizeY; //16

	uint8_t WhichFont;
	uint8_t Length;
	uint16_t Duration;

	uint16_t PositionZ;
	uint8_t BreakSizeX;
	uint8_t BreakSizeY;

	uint8_t FontColor;
	uint8_t Separator1[7]; //32

	uint8_t ReservedSpace[16];

	char ActualText[StoryBoardMaximumTextSize];

} StoryBoardTextStruct;

typedef struct {

	uint8_t SoundEffectID;
	uint8_t SoundEffectAction;
	uint8_t SoundEffectNameID;
	uint8_t Separator1;

	uint16_t SoundEffectPlayTime;


	uint8_t Separator2[10];

	uint8_t ReservedSpace[16];

} StoryBoardSoundEffectStruct;

#pragma pack(pop)

// TODO: REFACTOR ENDS HERE

typedef void(*StoryboardFinishedCB)(void* tCaller);

fup void setupStoryboards();
fup void shutdownStoryboards();
fup void updateStoryboards();

fup int playStoryboard(char* tPath);
fup void setStoryboardFinishedCB(int tID, StoryboardFinishedCB tCB, void* tCaller);

fup int isStoryboard(char* tPath);
