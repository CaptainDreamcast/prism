#include "loader.h"

// TODO: REFACTOR

#include "../../include/storyboard.h"
#include "../common/legacydefinitions.h"
#include "../common/scriptreader.h"

#include "media.h"

long LoadStoryBoardData(FILE* InputFile, uint32 OverallStartOffset, char TextureInputFolder[], char FinalOutputFileDir[], char RomDiskOutputFileDir[], char ConverterDirectory[], char CompressorDirectory[], char CharacterName[], uint8* WhichStoryBoard) {

	StoryBoardUberHeaderStruct StoryBoardUberHeaderData;
	StoryBoardHeaderStruct StoryBoardHeaderData;
	StoryBoardTextureStruct StoryBoardTextureData;
	StoryBoardTextStruct StoryBoardTextData;
	StoryBoardSoundEffectStruct StoryBoardSoundEffectData;

	FILE* OutputFile;
	int CurrentOffset;
	uint32 OverallEndOffset;

	uint32 StartOffset;
	uint32 EndOffset;

	uint32 RoundStartOffset;
	uint32 RoundEndOffset;

	uint8 ReadyToRock;
	uint8 AnotherRound;
	uint8 BurningDownTheHouse;
	uint8 RecalculateDuration;

	uint8 SoundEffectAmount;
	uint8 TextureAmount;
	uint8 CopyOverTexture;

	short SixteenBitBuffer;

	char FileDir[FILENAME_MAX];

	char TemporaryTextureName[NormalNameSize];
	char ActualTextBuffer[StoryBoardMaximumTextSize];

	char SearchForThis[CheckTextArraySize];
	char CheckText[CheckTextArraySize];
	char CompareWithThis[CheckTextArraySize];

	char StructType[StoryBoardStructAmount][CheckTextArraySize];

	sprintf(StructType[StoryBoardTextureStructIdentifier], "TEXTURE");
	sprintf(StructType[StoryBoardTextStructIdentifier], "TEXT");
	sprintf(StructType[StoryBoardSoundEffectStructIdentifier], "SOUND EFFECT");


	SoundEffectAmount = 0;
	TextureAmount = 0;

	CurrentOffset = OverallStartOffset;

	fseek(InputFile, 0, SEEK_END);
	OverallEndOffset = ftell(InputFile);

	//--------------------------------------------------------

	sprintf(SearchForThis, "CHARACTER NAME =");
	BurningDownTheHouse = SearchForString(InputFile, SearchForThis, &CurrentOffset, OverallStartOffset, OverallEndOffset);

	sprintf(SearchForThis, "[");
	BurningDownTheHouse = SearchForString(InputFile, SearchForThis, &CurrentOffset, OverallStartOffset, OverallEndOffset);

	ReadArrayFromInputFile(InputFile, CurrentOffset, CheckText, 0);
	sprintf(CharacterName, "%s", CheckText);
	printf("CharacterName: (%s)\n", CharacterName);

	//--------------------------------------------------------

	sprintf(SearchForThis, "WHICH STORYBOARD =");
	BurningDownTheHouse = SearchForString(InputFile, SearchForThis, &CurrentOffset, OverallStartOffset, OverallEndOffset);

	sprintf(SearchForThis, "[");
	BurningDownTheHouse = SearchForString(InputFile, SearchForThis, &CurrentOffset, OverallStartOffset, OverallEndOffset);

	*WhichStoryBoard = (uint8)ReadNumberFromInputFile(InputFile, CurrentOffset);
	printf("WhichStoryBoard: (%d)\n", *WhichStoryBoard);

	//--------------------------------------------------------

	sprintf(FileDir, "NAMES.tmp");
	OutputFile = fopen(FileDir, "wb+");
	fclose(OutputFile);

	sprintf(FileDir, "SOUNDEFFECTS.tmp");
	OutputFile = fopen(FileDir, "wb+");
	fclose(OutputFile);


	sprintf(FileDir, "%s/STORYBOARD", FinalOutputFileDir);
	OutputFile = fopen(FileDir, "wb+");

	StoryBoardUberHeaderData.ActionAmount = 0;
	fseek(OutputFile, 0, SEEK_SET);
	fwrite(&StoryBoardUberHeaderData, sizeof(StoryBoardUberHeaderStruct), 1, OutputFile);

	sprintf(SearchForThis, "###");
	BurningDownTheHouse = SearchForString(InputFile, SearchForThis, &CurrentOffset, OverallStartOffset, OverallEndOffset);
	RoundEndOffset = CurrentOffset;

	AnotherRound = 1;
	while (AnotherRound) {

		RoundStartOffset = RoundEndOffset;
		CurrentOffset = RoundEndOffset + 1;


		sprintf(SearchForThis, "###");
		BurningDownTheHouse = SearchForString(InputFile, SearchForThis, &CurrentOffset, OverallStartOffset, OverallEndOffset);
		RoundEndOffset = CurrentOffset;

		CurrentOffset = RoundStartOffset + 1;
		if (CurrentOffset > (int)RoundEndOffset) { RoundEndOffset = OverallEndOffset; AnotherRound = 0; }


		//--------------------------------------------------------

		StoryBoardHeaderData.WhichAction = StoryBoardUberHeaderData.ActionAmount + 1;

		//--------------------------------------------------------

		StoryBoardHeaderData.WaitForButtonInput = 0;

		//--------------------------------------------------------

		sprintf(SearchForThis, "ACTION DURATION =");
		if (SearchForString(InputFile, SearchForThis, &CurrentOffset, RoundStartOffset, RoundEndOffset)) {

			sprintf(SearchForThis, "[");
			BurningDownTheHouse = SearchForString(InputFile, SearchForThis, &CurrentOffset, RoundStartOffset, RoundEndOffset);

			StoryBoardHeaderData.Duration = (uint16_t)ReadNumberFromInputFile(InputFile, CurrentOffset);
			RecalculateDuration = 0;
		}
		else {
			StoryBoardHeaderData.Duration = 0;
			RecalculateDuration = 1;
		}
		printf("Duration: (%d)\n", StoryBoardHeaderData.Duration);

		//--------------------------------------------------------

		sprintf(SearchForThis, "SOUND TRACK =");
		if (SearchForString(InputFile, SearchForThis, &CurrentOffset, RoundStartOffset, RoundEndOffset)) {

			sprintf(SearchForThis, "[");
			BurningDownTheHouse = SearchForString(InputFile, SearchForThis, &CurrentOffset, RoundStartOffset, RoundEndOffset);

			StoryBoardHeaderData.SoundTrack = (uint8_t)ReadNumberFromInputFile(InputFile, CurrentOffset);
		}
		else StoryBoardHeaderData.SoundTrack = 0;

		printf("SoundTrack: (%d)\n", StoryBoardHeaderData.SoundTrack);

		//--------------------------------------------------------

		StoryBoardHeaderData.TextureStructAmount = 0;

		//--------------------------------------------------------

		StoryBoardHeaderData.TextStructAmount = 0;

		//--------------------------------------------------------

		StoryBoardHeaderData.SoundEffectStructAmount = 0;

		//--------------------------------------------------------

		fseek(OutputFile, 0, SEEK_END);
		fwrite(&StoryBoardHeaderData, sizeof(StoryBoardHeaderStruct), 1, OutputFile);

		EndOffset = RoundStartOffset;

		ReadyToRock = 1;

		while (ReadyToRock) {

			StartOffset = EndOffset;
			CurrentOffset = EndOffset + 1;


			sprintf(SearchForThis, "---");
			if (SearchForString(InputFile, SearchForThis, &CurrentOffset, RoundStartOffset, RoundEndOffset)) EndOffset = CurrentOffset;
			else EndOffset = StartOffset;

			CurrentOffset = StartOffset + 1;
			if (CurrentOffset > (int)EndOffset) { EndOffset = RoundEndOffset; ReadyToRock = 0; }

			//--------------------------------------------------------

			sprintf(SearchForThis, "INFO TYPE =");
			BurningDownTheHouse = SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset);

			sprintf(SearchForThis, "[");
			BurningDownTheHouse = SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset);

			ReadArrayFromInputFile(InputFile, CurrentOffset, CheckText, 0);
			printf("StructType [ARRAY]: (%s)\n", CheckText);

			if (CompareStrings(StructType[StoryBoardTextureStructIdentifier], CheckText)) {


				//--------------------------------------------------------

				sprintf(SearchForThis, "TEXTURE ID =");
				BurningDownTheHouse = SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset);

				sprintf(SearchForThis, "[");
				BurningDownTheHouse = SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset);

				StoryBoardTextureData.TextureID = (uint8_t)ReadNumberFromInputFile(InputFile, CurrentOffset);
				printf("TextureID: (%d)\n", StoryBoardTextureData.TextureID);

				//--------------------------------------------------------

				sprintf(SearchForThis, "TEXTURE ACTION =");
				BurningDownTheHouse = SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset);

				sprintf(SearchForThis, "[");
				BurningDownTheHouse = SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset);

				ReadArrayFromInputFile(InputFile, CurrentOffset, CheckText, 0);
				printf("TextureAction [ARRAY]: (%s)\n", CheckText);

				StoryBoardTextureData.TextureAction = 0xFF;

				sprintf(CompareWithThis, "DESTROY");
				if (CompareStrings(CompareWithThis, CheckText)) StoryBoardTextureData.TextureAction = StoryBoardDestroyTextureIdentifier;

				sprintf(CompareWithThis, "LOAD");
				if (CompareStrings(CompareWithThis, CheckText)) StoryBoardTextureData.TextureAction = StoryBoardLoadTextureIdentifier;

				if (StoryBoardTextureData.TextureAction == 0xFF) StoryBoardTextureData.TextureAction = StoryBoardChangeTextureIdentifier;

				printf("TextureAction [VALUE]: (%d)\n", StoryBoardTextureData.TextureAction);

				//--------------------------------------------------------

				sprintf(SearchForThis, "WHICH TEXTURE FRAME =");
				if (SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset)) {

					sprintf(SearchForThis, "[");
					BurningDownTheHouse = SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset);

					StoryBoardTextureData.WhichFrame = (uint8_t)ReadNumberFromInputFile(InputFile, CurrentOffset);
				}
				else StoryBoardTextureData.WhichFrame = 0;

				printf("WhichFrame: (%d)\n", StoryBoardTextureData.WhichFrame);

				//--------------------------------------------------------

				sprintf(SearchForThis, "TEXTURE FRAME AMOUNT =");
				if (SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset)) {

					sprintf(SearchForThis, "[");
					BurningDownTheHouse = SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset);

					StoryBoardTextureData.FrameAmount = (uint8_t)ReadNumberFromInputFile(InputFile, CurrentOffset);
				}
				else StoryBoardTextureData.FrameAmount = 1;

				printf("FrameAmount: (%d)\n", StoryBoardTextureData.FrameAmount);

				//--------------------------------------------------------

				sprintf(SearchForThis, "TEXTURE SPEED =");
				if (SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset)) {

					sprintf(SearchForThis, "[");
					BurningDownTheHouse = SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset);

					StoryBoardTextureData.Speed = (uint8_t)ReadNumberFromInputFile(InputFile, CurrentOffset);
				}
				else StoryBoardTextureData.Speed = 1;

				printf("Speed: (%d)\n", StoryBoardTextureData.Speed);

				//--------------------------------------------------------

				sprintf(SearchForThis, "TEXTURE LOOP =");
				if (SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset)) {

					sprintf(SearchForThis, "[");
					BurningDownTheHouse = SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset);

					ReadArrayFromInputFile(InputFile, CurrentOffset, CheckText, 0);
					printf("Loop [ARRAY]: (%s)\n", CheckText);

					sprintf(CompareWithThis, "YES");
					if (CompareStrings(CompareWithThis, CheckText)) StoryBoardTextureData.Loop = 1;
					else StoryBoardTextureData.Loop = 0;
				}
				else StoryBoardTextureData.Loop = 0;

				printf("Loop [VALUE]: (%d)\n", StoryBoardTextureData.Loop);

				//--------------------------------------------------------

				sprintf(SearchForThis, "TEXTURE POSITION X-AXIS =");
				if (SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset)) {

					sprintf(SearchForThis, "[");
					BurningDownTheHouse = SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset);

					StoryBoardTextureData.PositionX = (uint16_t)ReadNumberFromInputFile(InputFile, CurrentOffset);
				}
				else StoryBoardTextureData.PositionX = 0;

				printf("PositionX: (%d)\n", StoryBoardTextureData.PositionX);

				//--------------------------------------------------------

				sprintf(SearchForThis, "TEXTURE POSITION Y-AXIS =");
				if (SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset)) {

					sprintf(SearchForThis, "[");
					BurningDownTheHouse = SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset);

					StoryBoardTextureData.PositionY = (uint16_t)ReadNumberFromInputFile(InputFile, CurrentOffset);
				}
				else StoryBoardTextureData.PositionY = 0;

				printf("PositionY: (%d)\n", StoryBoardTextureData.PositionY);

				//--------------------------------------------------------

				sprintf(SearchForThis, "TEXTURE POSITION Z-AXIS =");
				if (SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset)) {

					sprintf(SearchForThis, "[");
					BurningDownTheHouse = SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset);

					StoryBoardTextureData.PositionZ = (uint16_t)ReadNumberFromInputFile(InputFile, CurrentOffset);
				}
				else StoryBoardTextureData.PositionZ = 0;

				printf("PositionZ: (%d)\n", StoryBoardTextureData.PositionZ);

				//--------------------------------------------------------

				sprintf(SearchForThis, "TEXTURE SIZE X-AXIS =");
				if (SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset)) {

					sprintf(SearchForThis, "[");
					BurningDownTheHouse = SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset);

					StoryBoardTextureData.SizeX = (uint16_t)ReadNumberFromInputFile(InputFile, CurrentOffset);
				}
				else StoryBoardTextureData.SizeX = 0;

				printf("SizeX: (%d)\n", StoryBoardTextureData.SizeX);

				//--------------------------------------------------------

				sprintf(SearchForThis, "TEXTURE SIZE Y-AXIS =");
				if (SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset)) {

					sprintf(SearchForThis, "[");
					BurningDownTheHouse = SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset);

					StoryBoardTextureData.SizeY = (uint16_t)ReadNumberFromInputFile(InputFile, CurrentOffset);
				}
				else StoryBoardTextureData.SizeY = 0;

				printf("SizeY: (%d)\n", StoryBoardTextureData.SizeY);

				//--------------------------------------------------------

				sprintf(SearchForThis, "TEXTURE MOVEMENT X-AXIS =");
				if (SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset)) {

					sprintf(SearchForThis, "[");
					BurningDownTheHouse = SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset);

					StoryBoardTextureData.MovementX = (short)ReadNumberFromInputFile(InputFile, CurrentOffset);
				}
				else StoryBoardTextureData.MovementX = 0;

				printf("MovementX: (%d)\n", StoryBoardTextureData.MovementX);

				//--------------------------------------------------------

				sprintf(SearchForThis, "TEXTURE MOVEMENT Y-AXIS =");
				if (SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset)) {

					sprintf(SearchForThis, "[");
					BurningDownTheHouse = SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset);

					StoryBoardTextureData.MovementY = (short)ReadNumberFromInputFile(InputFile, CurrentOffset);
				}
				else StoryBoardTextureData.MovementY = 0;

				printf("MovementY: (%d)\n", StoryBoardTextureData.MovementY);

				//--------------------------------------------------------

				sprintf(SearchForThis, "TEXTURE REAL SIZE X-AXIS =");
				if (SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset)) {

					sprintf(SearchForThis, "[");
					BurningDownTheHouse = SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset);

					StoryBoardTextureData.TextureSizeX = ReadNumberFromInputFile(InputFile, CurrentOffset);
				}
				else StoryBoardTextureData.TextureSizeX = 0;

				printf("TextureSizeX: (%d)\n", StoryBoardTextureData.TextureSizeX);

				//--------------------------------------------------------

				sprintf(SearchForThis, "TEXTURE REAL SIZE Y-AXIS =");
				if (SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset)) {

					sprintf(SearchForThis, "[");
					BurningDownTheHouse = SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset);

					StoryBoardTextureData.TextureSizeY = ReadNumberFromInputFile(InputFile, CurrentOffset);
				}
				else StoryBoardTextureData.TextureSizeY = 0;

				printf("TextureSizeY: (%d)\n", StoryBoardTextureData.TextureSizeY);

				//--------------------------------------------------------

				sprintf(SearchForThis, "LEFT INTERNAL TEXTURE POSITION =");
				if (SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset)) {

					sprintf(SearchForThis, "[");
					BurningDownTheHouse = SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset);

					SixteenBitBuffer = (short)ReadNumberFromInputFile(InputFile, CurrentOffset);
					StoryBoardTextureData.TexturePositionX1 = (SixteenBitBuffer / 100.0f);

				}
				else StoryBoardTextureData.TexturePositionX1 = 0;

				printf("TexturePositionX1: (%f)\n", StoryBoardTextureData.TexturePositionX1);

				//--------------------------------------------------------

				sprintf(SearchForThis, "UPPER INTERNAL TEXTURE POSITION =");
				if (SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset)) {

					sprintf(SearchForThis, "[");
					BurningDownTheHouse = SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset);

					SixteenBitBuffer = (short)ReadNumberFromInputFile(InputFile, CurrentOffset);
					StoryBoardTextureData.TexturePositionY1 = (SixteenBitBuffer / 100.0f);

				}
				else StoryBoardTextureData.TexturePositionY1 = 0;

				printf("TexturePositionY1: (%f)\n", StoryBoardTextureData.TexturePositionY1);

				//--------------------------------------------------------

				sprintf(SearchForThis, "RIGHT INTERNAL TEXTURE POSITION =");
				if (SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset)) {

					sprintf(SearchForThis, "[");
					BurningDownTheHouse = SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset);

					SixteenBitBuffer = (short)ReadNumberFromInputFile(InputFile, CurrentOffset);
					StoryBoardTextureData.TexturePositionX2 = (SixteenBitBuffer / 100.0f);

				}
				else StoryBoardTextureData.TexturePositionX2 = 0;

				printf("TexturePositionX2: (%f)\n", StoryBoardTextureData.TexturePositionX2);

				//--------------------------------------------------------

				sprintf(SearchForThis, "LOWER INTERNAL TEXTURE POSITION =");
				if (SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset)) {

					sprintf(SearchForThis, "[");
					BurningDownTheHouse = SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset);

					SixteenBitBuffer = (short)ReadNumberFromInputFile(InputFile, CurrentOffset);
					StoryBoardTextureData.TexturePositionY2 = (SixteenBitBuffer / 100.0f);

				}
				else StoryBoardTextureData.TexturePositionY2 = 0;

				printf("TexturePositionY2: (%f)\n", StoryBoardTextureData.TexturePositionY2);

				//--------------------------------------------------------

				sprintf(SearchForThis, "TEXTURE NAME =");
				BurningDownTheHouse = SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset);

				sprintf(SearchForThis, "[");
				BurningDownTheHouse = SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset);

				ReadArrayFromInputFile(InputFile, CurrentOffset, CheckText, 0);
				sprintf(TemporaryTextureName, "%s", CheckText);
				printf("TextureName: (%s)\n", TemporaryTextureName);

				CopyOverTexture = CheckIfCopyingIsNecessary(TemporaryTextureName, &StoryBoardTextureData.TextureNameID, &TextureAmount);

				//--------------------------------------------------------


				if (CopyOverTexture) CopyOverTextureFunction(TemporaryTextureName, FinalOutputFileDir, TextureInputFolder, StoryBoardTextureData.TextureNameID, StoryBoardTextureData.FrameAmount, ConverterDirectory, CompressorDirectory);

				CurrentOffset = StoryBoardHeaderData.SoundEffectStructAmount * sizeof(StoryBoardSoundEffectStruct) + StoryBoardHeaderData.TextStructAmount * sizeof(StoryBoardTextStruct);
				fseek(OutputFile, -CurrentOffset, SEEK_END);
				fwrite(&StoryBoardTextureData, sizeof(StoryBoardTextureStruct), 1, OutputFile);

				StoryBoardHeaderData.TextureStructAmount++;
			}
			else if (CompareStrings(StructType[StoryBoardTextStructIdentifier], CheckText)) {

				//--------------------------------------------------------

				sprintf(SearchForThis, "TEXT ID =");
				BurningDownTheHouse = SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset);

				sprintf(SearchForThis, "[");
				BurningDownTheHouse = SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset);

				StoryBoardTextData.TextID = (uint8_t)ReadNumberFromInputFile(InputFile, CurrentOffset);
				printf("TextID: (%d)\n", StoryBoardTextData.TextID);

				//--------------------------------------------------------

				sprintf(SearchForThis, "TEXT ACTION =");
				BurningDownTheHouse = SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset);

				sprintf(SearchForThis, "[");
				BurningDownTheHouse = SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset);

				ReadArrayFromInputFile(InputFile, CurrentOffset, CheckText, 0);
				printf("TextAction [ARRAY]: (%s)\n", CheckText);

				sprintf(CompareWithThis, "DESTROY");
				if (CompareStrings(CompareWithThis, CheckText)) StoryBoardTextData.TextAction = StoryBoardDestroyTextIdentifier;
				else StoryBoardTextData.TextAction = 1;

				printf("TextAction [VALUE]: (%d)\n", StoryBoardTextData.TextAction);

				//--------------------------------------------------------

				sprintf(SearchForThis, "TEXT FONT COLOR =");
				BurningDownTheHouse = SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset);

				sprintf(SearchForThis, "[");
				BurningDownTheHouse = SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset);

				ReadArrayFromInputFile(InputFile, CurrentOffset, CheckText, 0);
				printf("FontColor [ARRAY]: (%s)\n", CheckText);

				StoryBoardTextData.FontColor = WhiteColor;

				sprintf(CompareWithThis, "BLACK");
				if (CompareStrings(CompareWithThis, CheckText)) StoryBoardTextData.FontColor = BlackColor;

				sprintf(CompareWithThis, "RED");
				if (CompareStrings(CompareWithThis, CheckText)) StoryBoardTextData.FontColor = RedColor;

				sprintf(CompareWithThis, "GREEN");
				if (CompareStrings(CompareWithThis, CheckText)) StoryBoardTextData.FontColor = GreenColor;

				sprintf(CompareWithThis, "BLUE");
				if (CompareStrings(CompareWithThis, CheckText)) StoryBoardTextData.FontColor = BlueColor;

				sprintf(CompareWithThis, "YELLOW");
				if (CompareStrings(CompareWithThis, CheckText)) StoryBoardTextData.FontColor = YellowColor;

				sprintf(CompareWithThis, "WHITE");
				if (CompareStrings(CompareWithThis, CheckText)) StoryBoardTextData.FontColor = WhiteColor;

				printf("FontColor [VALUE]: (%d)\n", StoryBoardTextData.FontColor);

				//--------------------------------------------------------

				sprintf(SearchForThis, "TEXT BUILD UP =");
				BurningDownTheHouse = SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset);

				sprintf(SearchForThis, "[");
				BurningDownTheHouse = SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset);

				ReadArrayFromInputFile(InputFile, CurrentOffset, CheckText, 0);
				printf("BuildUp [ARRAY]: (%s)\n", CheckText);

				sprintf(CompareWithThis, "YES");
				if (CompareStrings(CompareWithThis, CheckText)) StoryBoardTextData.BuildUp = 1;
				else StoryBoardTextData.BuildUp = 0;

				printf("BuildUp [VALUE]: (%d)\n", StoryBoardTextData.BuildUp);

				//--------------------------------------------------------

				sprintf(SearchForThis, "TEXT BUILD UP SPEED =");
				if (SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset)) {

					sprintf(SearchForThis, "[");
					BurningDownTheHouse = SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset);

					StoryBoardTextData.BuildUpSpeed = (uint8_t)ReadNumberFromInputFile(InputFile, CurrentOffset);
				}
				else StoryBoardTextData.BuildUpSpeed = 1;

				printf("BuildUpSpeed: (%d)\n", StoryBoardTextData.BuildUpSpeed);

				//--------------------------------------------------------

				sprintf(SearchForThis, "TEXT POSITION X-AXIS =");
				if (SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset)) {

					sprintf(SearchForThis, "[");
					BurningDownTheHouse = SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset);

					StoryBoardTextData.PositionX = (uint16_t)ReadNumberFromInputFile(InputFile, CurrentOffset);
				}
				else StoryBoardTextData.PositionX = 1;

				printf("PositionX: (%d)\n", StoryBoardTextData.PositionX);

				//--------------------------------------------------------

				sprintf(SearchForThis, "TEXT POSITION Y-AXIS =");
				if (SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset)) {

					sprintf(SearchForThis, "[");
					BurningDownTheHouse = SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset);

					StoryBoardTextData.PositionY = (uint16_t)ReadNumberFromInputFile(InputFile, CurrentOffset);
				}
				else StoryBoardTextData.PositionY = 1;

				printf("PositionY: (%d)\n", StoryBoardTextData.PositionY);

				//--------------------------------------------------------

				sprintf(SearchForThis, "TEXT POSITION Z-AXIS =");
				if (SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset)) {

					sprintf(SearchForThis, "[");
					BurningDownTheHouse = SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset);

					StoryBoardTextData.PositionZ = (uint16_t)ReadNumberFromInputFile(InputFile, CurrentOffset);
				}
				else StoryBoardTextData.PositionZ = 1;

				printf("PositionZ: (%d)\n", StoryBoardTextData.PositionZ);

				//--------------------------------------------------------

				sprintf(SearchForThis, "TEXT SIZE X-AXIS =");
				if (SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset)) {

					sprintf(SearchForThis, "[");
					BurningDownTheHouse = SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset);

					StoryBoardTextData.SizeX = (uint16_t)ReadNumberFromInputFile(InputFile, CurrentOffset);
				}
				else StoryBoardTextData.SizeX = 1;

				printf("SizeX: (%d)\n", StoryBoardTextData.SizeX);

				//--------------------------------------------------------

				sprintf(SearchForThis, "TEXT SIZE Y-AXIS =");
				if (SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset)) {

					sprintf(SearchForThis, "[");
					BurningDownTheHouse = SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset);

					StoryBoardTextData.SizeY = (uint16_t)ReadNumberFromInputFile(InputFile, CurrentOffset);
				}
				else StoryBoardTextData.SizeY = ScreenSizeY;

				printf("SizeY: (%d)\n", StoryBoardTextData.SizeY);

				//--------------------------------------------------------

				sprintf(SearchForThis, "TEXT BREAK SIZE X-AXIS =");
				if (SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset)) {

					sprintf(SearchForThis, "[");
					BurningDownTheHouse = SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset);

					StoryBoardTextData.BreakSizeX = (uint8_t)ReadNumberFromInputFile(InputFile, CurrentOffset);
				}
				else StoryBoardTextData.BreakSizeX = 1;

				printf("BreakSizeX: (%d)\n", StoryBoardTextData.BreakSizeX);

				//--------------------------------------------------------

				sprintf(SearchForThis, "TEXT BREAK SIZE Y-AXIS =");
				if (SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset)) {

					sprintf(SearchForThis, "[");
					BurningDownTheHouse = SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset);

					StoryBoardTextData.BreakSizeY = (uint8_t)ReadNumberFromInputFile(InputFile, CurrentOffset);
				}
				else StoryBoardTextData.BreakSizeY = 1;

				printf("BreakSizeY: (%d)\n", StoryBoardTextData.BreakSizeY);

				//--------------------------------------------------------

				sprintf(SearchForThis, "TEXT FONT SIZE X-AXIS =");
				if (SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset)) {

					sprintf(SearchForThis, "[");
					BurningDownTheHouse = SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset);

					StoryBoardTextData.FontSizeX = (uint16_t)ReadNumberFromInputFile(InputFile, CurrentOffset);
				}
				else StoryBoardTextData.FontSizeX = 1;

				printf("FontSizeX: (%d)\n", StoryBoardTextData.FontSizeX);

				//--------------------------------------------------------

				sprintf(SearchForThis, "TEXT FONT SIZE Y-AXIS =");
				if (SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset)) {

					sprintf(SearchForThis, "[");
					BurningDownTheHouse = SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset);

					StoryBoardTextData.FontSizeY = (uint16_t)ReadNumberFromInputFile(InputFile, CurrentOffset);
				}
				else StoryBoardTextData.FontSizeY = 1;

				printf("FontSizeY: (%d)\n", StoryBoardTextData.FontSizeY);

				//--------------------------------------------------------

				sprintf(SearchForThis, "TEXT WHICH FONT =");
				if (SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset)) {

					sprintf(SearchForThis, "[");
					BurningDownTheHouse = SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset);

					StoryBoardTextData.WhichFont = (uint8_t)ReadNumberFromInputFile(InputFile, CurrentOffset);
				}
				else StoryBoardTextData.WhichFont = 1;

				printf("WhichFont: (%d)\n", StoryBoardTextData.WhichFont);

				//--------------------------------------------------------

				sprintf(SearchForThis, "TEXT DURATION =");
				if (SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset)) {

					sprintf(SearchForThis, "[");
					BurningDownTheHouse = SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset);

					StoryBoardTextData.Duration = (uint16_t)ReadNumberFromInputFile(InputFile, CurrentOffset);
				}
				else StoryBoardTextData.Duration = 1;

				printf("Duration: (%d)\n", StoryBoardTextData.Duration);

				//--------------------------------------------------------

				sprintf(SearchForThis, "ACTUAL TEXT =");
				BurningDownTheHouse = SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset);

				sprintf(SearchForThis, "[");
				BurningDownTheHouse = SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset);

				ReadArrayFromInputFile(InputFile, CurrentOffset, ActualTextBuffer, 0);
				StoryBoardTextData.Length = sprintf(StoryBoardTextData.ActualText, "%s", ActualTextBuffer);

				printf("Length: (%d)\n", StoryBoardTextData.Length);
				printf("ActualText: (%s)\n", StoryBoardTextData.ActualText);

				//--------------------------------------------------------

				if (RecalculateDuration && StoryBoardTextData.BuildUp) {
					RecalculateDuration = 0;
					StoryBoardHeaderData.Duration = StoryBoardTextData.Length*StoryBoardTextData.BuildUpSpeed;
					printf("StoryBoardHeaderData.Duration: (%d)\n", StoryBoardHeaderData.Duration);
				}

				//--------------------------------------------------------

				CurrentOffset = StoryBoardHeaderData.SoundEffectStructAmount * sizeof(StoryBoardSoundEffectStruct);
				fseek(OutputFile, -CurrentOffset, SEEK_END);
				fwrite(&StoryBoardTextData, sizeof(StoryBoardTextStruct), 1, OutputFile);

				StoryBoardHeaderData.TextStructAmount++;
			}
			else if (CompareStrings(StructType[StoryBoardSoundEffectStructIdentifier], CheckText)) {

				//--------------------------------------------------------

				sprintf(SearchForThis, "SOUND EFFECT ID =");
				BurningDownTheHouse = SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset);

				sprintf(SearchForThis, "[");
				BurningDownTheHouse = SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset);

				StoryBoardSoundEffectData.SoundEffectID = (uint8_t)ReadNumberFromInputFile(InputFile, CurrentOffset);
				printf("SoundEffectID: (%d)\n", StoryBoardSoundEffectData.SoundEffectID);

				//--------------------------------------------------------

				sprintf(SearchForThis, "SOUND EFFECT ACTION =");
				BurningDownTheHouse = SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset);

				sprintf(SearchForThis, "[");
				BurningDownTheHouse = SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset);

				ReadArrayFromInputFile(InputFile, CurrentOffset, CheckText, 0);
				printf("SoundEffectAction [ARRAY]: (%s)\n", CheckText);

				sprintf(CompareWithThis, "DESTROY");
				if (CompareStrings(CompareWithThis, CheckText)) StoryBoardSoundEffectData.SoundEffectAction = StoryBoardDestroySoundEffectIdentifier;
				else StoryBoardSoundEffectData.SoundEffectAction = 1;

				printf("SoundEffectAction [VALUE]: (%d)\n", StoryBoardSoundEffectData.SoundEffectAction);

				//--------------------------------------------------------

				sprintf(SearchForThis, "SOUND EFFECT NAME =");
				BurningDownTheHouse = SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset);

				sprintf(SearchForThis, "[");
				BurningDownTheHouse = SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset);

				ReadArrayFromInputFile(InputFile, CurrentOffset, CheckText, 0);
				sprintf(TemporaryTextureName, "%s", CheckText);
				printf("SoundEffectName: (%s)\n", TemporaryTextureName);

				CopyOverTexture = CheckIfCopyingSoundEffectIsNecessary(TemporaryTextureName, &StoryBoardSoundEffectData.SoundEffectNameID, &SoundEffectAmount);

				//--------------------------------------------------------

				sprintf(SearchForThis, "SOUND EFFECT PLAY TIME =");
				BurningDownTheHouse = SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset);

				sprintf(SearchForThis, "[");
				BurningDownTheHouse = SearchForString(InputFile, SearchForThis, &CurrentOffset, StartOffset, EndOffset);

				StoryBoardSoundEffectData.SoundEffectPlayTime = (uint16_t)ReadNumberFromInputFile(InputFile, CurrentOffset);
				printf("SoundEffectPlayTime: (%d)\n", StoryBoardSoundEffectData.SoundEffectPlayTime);

				//--------------------------------------------------------

				fseek(OutputFile, 0, SEEK_END);
				fwrite(&StoryBoardSoundEffectData, sizeof(StoryBoardSoundEffectStruct), 1, OutputFile);

				if (CopyOverTexture) CopyOverSoundEffectFunction(TemporaryTextureName, FinalOutputFileDir, TextureInputFolder, StoryBoardSoundEffectData.SoundEffectNameID);

				StoryBoardHeaderData.SoundEffectStructAmount++;
			}


			printf("\n");
		}

		if (RecalculateDuration) StoryBoardHeaderData.Duration = 30000;

		CurrentOffset = sizeof(StoryBoardHeaderStruct) + StoryBoardHeaderData.SoundEffectStructAmount * sizeof(StoryBoardSoundEffectStruct) + StoryBoardHeaderData.TextStructAmount * sizeof(StoryBoardTextStruct) + StoryBoardHeaderData.TextureStructAmount * sizeof(StoryBoardTextureStruct);
		printf("Offset: (%ld)\n", CurrentOffset);

		fseek(OutputFile, -CurrentOffset, SEEK_END);
		fwrite(&StoryBoardHeaderData, sizeof(StoryBoardHeaderStruct), 1, OutputFile);

		StoryBoardUberHeaderData.ActionAmount++;
	}

	fseek(OutputFile, 0, SEEK_SET);
	fwrite(&StoryBoardUberHeaderData, sizeof(StoryBoardUberHeaderStruct), 1, OutputFile);

	fclose(OutputFile);

	return(OverallEndOffset);

}