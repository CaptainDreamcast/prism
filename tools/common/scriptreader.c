// TODO: Refactor

#include "scriptreader.h"

#include "legacydefinitions.h"

uint8 SearchForString(FILE* InputFile, char StringThatIsSearched[], uint32* InputOffset, uint32 StartOffset, uint32 EndOffset) {

	char CheckText[CheckTextArraySize];
	uint8 ReadyToRock;
	uint8 LoopDiLoop;
	char CheckArray[CheckTextArraySize];
	uint32 CheckArraySize;
	uint32 DirtyDeedsDoneDirtCheap;
	uint32 CurrentOffset;

	CurrentOffset = *InputOffset;

	CheckArraySize = sprintf(CheckArray, "%s", StringThatIsSearched);
	//CurrentOffset = StartOffset;
	LoopDiLoop = 0;
	ReadyToRock = 1;

	while (ReadyToRock) {

		ReadyToRock = 0;
		fseek(InputFile, CurrentOffset, SEEK_SET);
		fread(CheckText, 1, CheckArraySize, InputFile);

		for (DirtyDeedsDoneDirtCheap = 0; DirtyDeedsDoneDirtCheap < CheckArraySize; DirtyDeedsDoneDirtCheap++) {
			if (CheckText[DirtyDeedsDoneDirtCheap] != CheckArray[DirtyDeedsDoneDirtCheap]) ReadyToRock = 1;
		}

		if (ReadyToRock) CurrentOffset++;

		if (CurrentOffset > EndOffset) {
			if (LoopDiLoop == 1) return(0);
			CurrentOffset = StartOffset;
			LoopDiLoop = 1;
		}

	}

	*InputOffset = CurrentOffset;
	return(1);
}

void ReadArrayFromInputFile(FILE* InputFile, uint32 CurrentOffset, char DestinationArray[], uint8 DesiredArrayLength) {

	char CheckText[CheckTextArraySize];
	uint8 ReadyToRock;
	char CheckArray[CheckTextArraySize];
	long CheckArraySize;
	long DirtyDeedsDoneDirtCheap;
	long ArrayLength;


	fseek(InputFile, CurrentOffset, SEEK_SET);
	fread(CheckText, 1, 1, InputFile);
	CheckArraySize = sprintf(CheckArray, "[");
	if (CheckText[0] == CheckArray[0]) CurrentOffset++;


	ArrayLength = 0;
	CheckArraySize = sprintf(CheckArray, "]");
	ReadyToRock = 1;

	while (ReadyToRock) {

		ReadyToRock = 0;
		fseek(InputFile, CurrentOffset, SEEK_SET);
		fread(CheckText, 1, CheckArraySize, InputFile);

		for (DirtyDeedsDoneDirtCheap = 0; DirtyDeedsDoneDirtCheap < CheckArraySize; DirtyDeedsDoneDirtCheap++) {
			if (CheckText[DirtyDeedsDoneDirtCheap] != CheckArray[DirtyDeedsDoneDirtCheap]) ReadyToRock = 1;
		}

		if (ReadyToRock) { CurrentOffset++; ArrayLength++; }
	}

	CurrentOffset -= ArrayLength;
	fseek(InputFile, CurrentOffset, SEEK_SET);
	fread(CheckText, 1, ArrayLength, InputFile);

	if (DesiredArrayLength != 0) {
		while (ArrayLength <= DesiredArrayLength) {
			CheckText[ArrayLength] = 0x20;
			ArrayLength++;
		}
	}
	else DesiredArrayLength = (uint8)ArrayLength;

	CheckText[DesiredArrayLength] = '\0';

	sprintf(DestinationArray, "%s", CheckText);

}

long ReadNumberFromInputFile(FILE* InputFile, uint32 CurrentOffset) {

	char CheckText[CheckTextArraySize];
	uint8 ReadyToRock;
	char CheckArray[CheckTextArraySize];
	long CheckArraySize;
	long DirtyDeedsDoneDirtCheap;
	long ArrayLength;
	int Multiplier;
	int DestinationNumber;
	long Bluff;
	uint8 Minus;


	fseek(InputFile, CurrentOffset, SEEK_SET);
	fread(CheckText, 1, 1, InputFile);
	CheckArraySize = sprintf(CheckArray, "[");
	if (CheckText[0] == CheckArray[0]) CurrentOffset++;

	fseek(InputFile, CurrentOffset, SEEK_SET);
	fread(CheckText, 1, 1, InputFile);
	CheckArraySize = sprintf(CheckArray, "-");
	if (CheckText[0] == CheckArray[0]) { CurrentOffset++; Minus = 1; }
	else Minus = 0;

	ArrayLength = 0;
	CheckArraySize = sprintf(CheckArray, "]");
	ReadyToRock = 1;

	while (ReadyToRock) {

		ReadyToRock = 0;
		fseek(InputFile, CurrentOffset, SEEK_SET);
		fread(CheckText, 1, CheckArraySize, InputFile);

		for (DirtyDeedsDoneDirtCheap = 0; DirtyDeedsDoneDirtCheap < CheckArraySize; DirtyDeedsDoneDirtCheap++) {
			if (CheckText[DirtyDeedsDoneDirtCheap] != CheckArray[DirtyDeedsDoneDirtCheap]) ReadyToRock = 1;
		}

		if (ReadyToRock) { CurrentOffset++; ArrayLength++; }
	}



	CurrentOffset -= ArrayLength;
	fseek(InputFile, CurrentOffset, SEEK_SET);
	fread(CheckText, 1, ArrayLength, InputFile);
	CheckText[ArrayLength] = '\0';

	DestinationNumber = 0;
	DirtyDeedsDoneDirtCheap = 1;

	ArrayLength--;
	while (ArrayLength >= 0) {
		Multiplier = 1;
		Bluff = DirtyDeedsDoneDirtCheap;
		while (Bluff > 1) {
			Multiplier *= 10;
			Bluff--;
		}

		DestinationNumber += (CheckText[ArrayLength] - 48)*Multiplier;

		DirtyDeedsDoneDirtCheap++;
		ArrayLength--;
	}

	if (Minus) DestinationNumber = -DestinationNumber;

	return(DestinationNumber);
}

uint8 CompareStrings(char FirstArray[], char SecondArray[]) {

	uint32 CheckArraySize;
	char CheckArray[CheckTextArraySize];
	uint32 DirtyDeedsDoneDirtCheap;
	uint8 ArrayEquality;

	CheckArraySize = sprintf(CheckArray, "%s", FirstArray);

	ArrayEquality = 1;

	for (DirtyDeedsDoneDirtCheap = 0; DirtyDeedsDoneDirtCheap < CheckArraySize; DirtyDeedsDoneDirtCheap++) {
		if (CheckArray[DirtyDeedsDoneDirtCheap] != SecondArray[DirtyDeedsDoneDirtCheap]) ArrayEquality = 0;
	}

	return(ArrayEquality);
}

uint8 ReadButtonsFromInputFile(FILE* InputFile, uint32 CurrentOffset) {

	char CheckText[CheckTextArraySize];
	uint8 ReadyToRock;
	char CheckArray[CheckTextArraySize];
	long CheckArraySize;
	long DirtyDeedsDoneDirtCheap;
	long ArrayLength;
	uint8 WhichAttackID;


	fseek(InputFile, CurrentOffset, SEEK_SET);
	fread(CheckText, 1, 1, InputFile);
	CheckArraySize = sprintf(CheckArray, "[");
	if (CheckText[0] == CheckArray[0]) CurrentOffset++;

	CheckArraySize = sprintf(CheckArray, "X");

	fseek(InputFile, CurrentOffset, SEEK_SET);
	fread(CheckText, 1, CheckArraySize, InputFile);

	WhichAttackID = 0;

	CheckArraySize = sprintf(CheckArray, "A");
	if (CheckText[0] == CheckArray[0]) WhichAttackID = 1;

	CheckArraySize = sprintf(CheckArray, "B");
	if (CheckText[0] == CheckArray[0]) WhichAttackID = 11;

	CheckArraySize = sprintf(CheckArray, "X");
	if (CheckText[0] == CheckArray[0]) WhichAttackID = 21;

	CheckArraySize = sprintf(CheckArray, "Y");
	if (CheckText[0] == CheckArray[0]) WhichAttackID = 31;

	CheckArraySize = sprintf(CheckArray, "L");
	if (CheckText[0] == CheckArray[0]) WhichAttackID = 41;

	CheckArraySize = sprintf(CheckArray, "R");
	if (CheckText[0] == CheckArray[0]) WhichAttackID = 51;

	CurrentOffset++;

	fseek(InputFile, CurrentOffset, SEEK_SET);
	fread(CheckText, 1, 1, InputFile);
	CheckArraySize = sprintf(CheckArray, "]");
	if (CheckText[0] == CheckArray[0]) return(WhichAttackID);

	CheckArraySize = sprintf(CheckArray, "+");
	if (CheckText[0] == CheckArray[0]) CurrentOffset++;


	ArrayLength = 0;
	CheckArraySize = sprintf(CheckArray, "]");
	ReadyToRock = 1;

	while (ReadyToRock) {

		ReadyToRock = 0;
		fseek(InputFile, CurrentOffset, SEEK_SET);
		fread(CheckText, 1, CheckArraySize, InputFile);

		for (DirtyDeedsDoneDirtCheap = 0; DirtyDeedsDoneDirtCheap < CheckArraySize; DirtyDeedsDoneDirtCheap++) {
			if (CheckText[DirtyDeedsDoneDirtCheap] != CheckArray[DirtyDeedsDoneDirtCheap]) ReadyToRock = 1;
		}

		CurrentOffset++;
		ArrayLength++;
	}

	CurrentOffset -= ArrayLength;
	fseek(InputFile, CurrentOffset, SEEK_SET);
	fread(CheckText, 1, ArrayLength, InputFile);

	CheckText[ArrayLength] = '\0';

	CheckArraySize = sprintf(CheckArray, "FORWARD]");
	if (CompareStrings(CheckText, CheckArray)) WhichAttackID += AnimationIDOffsetHoldingForward;

	CheckArraySize = sprintf(CheckArray, "BACK]");
	if (CompareStrings(CheckText, CheckArray)) WhichAttackID += AnimationIDOffsetHoldingBack;

	CheckArraySize = sprintf(CheckArray, "DOWN-DOWNFORWARD-FORWARD]");
	if (CompareStrings(CheckText, CheckArray)) WhichAttackID += AnimationIDOffsetDownDownForwardForward;

	CheckArraySize = sprintf(CheckArray, "DOWN-DOWNBACK-BACK]");
	if (CompareStrings(CheckText, CheckArray)) WhichAttackID += AnimationIDOffsetDownDownBackBack;

	CheckArraySize = sprintf(CheckArray, "FORWARD-DOWN-DOWNFORWARD]");
	if (CompareStrings(CheckText, CheckArray)) WhichAttackID += AnimationIDOffsetForwardDownDownForward;

	CheckArraySize = sprintf(CheckArray, "A]");
	if (CompareStrings(CheckText, CheckArray)) {
		if (WhichAttackID == 21) WhichAttackID += AnimationIDOffsetSameStrengthButtons;
	}

	CheckArraySize = sprintf(CheckArray, "B]");
	if (CompareStrings(CheckText, CheckArray)) {
		if (WhichAttackID == 31) WhichAttackID += AnimationIDOffsetSameStrengthButtons;
	}

	CheckArraySize = sprintf(CheckArray, "X]");
	if (CompareStrings(CheckText, CheckArray)) {
		if (WhichAttackID == 1) WhichAttackID += AnimationIDOffsetSameStrengthButtons;
	}

	CheckArraySize = sprintf(CheckArray, "Y]");
	if (CompareStrings(CheckText, CheckArray)) {
		if (WhichAttackID == 11) WhichAttackID += AnimationIDOffsetSameStrengthButtons;
	}

	CheckArraySize = sprintf(CheckArray, "L]");
	if (CompareStrings(CheckText, CheckArray)) {
		if (WhichAttackID == 51) WhichAttackID += AnimationIDOffsetSameStrengthButtons;
	}

	CheckArraySize = sprintf(CheckArray, "R]");
	if (CompareStrings(CheckText, CheckArray)) {
		if (WhichAttackID == 41) WhichAttackID += AnimationIDOffsetSameStrengthButtons;
	}


	return(WhichAttackID);
}


uint8 CheckIfCopyingIsNecessary(char TemporaryAnimationName[], uint8* FrameID, uint8* AnimationAmount) {

	FILE* TemporaryNameFile;
	char FileDir[FILENAME_MAX];
	char CheckText[NormalNameSize];
	uint8 CopyOverFightAnimation;
	uint32 BailOutValue;
	uint32 CurrentOffset;

	sprintf(FileDir, "NAMES.tmp");
	TemporaryNameFile = fopen(FileDir, "rb+");

	fseek(TemporaryNameFile, 0, SEEK_END);
	BailOutValue = ftell(TemporaryNameFile);

	fseek(TemporaryNameFile, 0, SEEK_SET);

	CopyOverFightAnimation = 1;

	CurrentOffset = 0;
	while (CurrentOffset < BailOutValue && CopyOverFightAnimation) {

		fread(CheckText, 1, ActualNameSize, TemporaryNameFile);

		if (CompareStrings(TemporaryAnimationName, CheckText)) {
			*FrameID = (uint8)(CurrentOffset / ActualNameSize);
			CopyOverFightAnimation = 0;
		}

		CurrentOffset += ActualNameSize;
	}

	if (CopyOverFightAnimation) {

		fseek(TemporaryNameFile, 0, SEEK_END);
		fwrite(TemporaryAnimationName, 1, ActualNameSize, TemporaryNameFile);
		*FrameID = *AnimationAmount;
		(*AnimationAmount)++;

	}

	fclose(TemporaryNameFile);

	return(CopyOverFightAnimation);
}