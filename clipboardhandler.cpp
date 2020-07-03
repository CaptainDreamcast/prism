#include "prism/clipboardhandler.h"

#include <string.h>

#include "prism/mugentexthandler.h"
#include "prism/log.h"
#include "prism/system.h"
#include "prism/geometry.h"

#define CLIPBOARD_LINE_AMOUNT 10

static struct {
	int mLineAmount;
	char mLines[CLIPBOARD_LINE_AMOUNT][1024];

	int mTextIDs[CLIPBOARD_LINE_AMOUNT];

	int mIsVisible;
} gPrismClipboardHandlerData;

void initClipboardForGame() {
	memset(gPrismClipboardHandlerData.mLines, 0, sizeof gPrismClipboardHandlerData.mLines);
	gPrismClipboardHandlerData.mLineAmount = 0;
	gPrismClipboardHandlerData.mIsVisible = 0;
}

static void initClipboardLines() {
	static const auto CLIPBOARD_FONT = -1;
	double deltaY = getMugenFontSizeY(CLIPBOARD_FONT) + getMugenFontSpacingY(CLIPBOARD_FONT);
	Position pos = Vector3D(20, 20, 90);
	int i;
	for (i = 0; i < CLIPBOARD_LINE_AMOUNT; i++) {
		gPrismClipboardHandlerData.mTextIDs[i] = addMugenText("", pos, CLIPBOARD_FONT);
		pos.y += deltaY;
	}
}

static void setClipboardLineTexts() {
	int i;
	for (i = 0; i < gPrismClipboardHandlerData.mLineAmount; i++) {
		if (gPrismClipboardHandlerData.mIsVisible) {
			changeMugenText(gPrismClipboardHandlerData.mTextIDs[i], gPrismClipboardHandlerData.mLines[i]);
		}
		else {
			changeMugenText(gPrismClipboardHandlerData.mTextIDs[i], "");
		}
	}
}

static void loadClipboardHandler(void* tData) {
	(void)tData;
	setProfilingSectionMarkerCurrentFunction();
	initClipboardLines();
	setClipboardLineTexts();
}

ActorBlueprint getClipboardHandler() {
	return makeActorBlueprint(loadClipboardHandler);
}

static void moveClipboardLinesDown() {
	int i;
	for (i = 1; i < gPrismClipboardHandlerData.mLineAmount; i++) {
		strcpy(gPrismClipboardHandlerData.mLines[i - 1], gPrismClipboardHandlerData.mLines[i]);
	}

	gPrismClipboardHandlerData.mLineAmount--;
}

void addClipboardLine(char* tLine) {
	if (gPrismClipboardHandlerData.mLineAmount == CLIPBOARD_LINE_AMOUNT) {
		moveClipboardLinesDown();
	}

	strcpy(gPrismClipboardHandlerData.mLines[gPrismClipboardHandlerData.mLineAmount], tLine);
	gPrismClipboardHandlerData.mLineAmount++;

	setClipboardLineTexts();
}

static void getArgumentTextAndAdvanceParams(char* tArgumentText, char** tParams) {
	if (*tParams == NULL) {
		tArgumentText[0] = '\0';
		return;
	}

	char* nextComma = strchr(*tParams, ',');
	if (nextComma) *nextComma = '\0';

	sprintf(tArgumentText, "%s", *tParams);

	*tParams = nextComma + 1;
}

static void parseParameterInput(const char* tFormatString, int* i, char** tDst, char** tParams) {

	(*i)++;
	while (isdigit(tFormatString[*i]) || tFormatString[*i] == '.') { // don't care about precision
		(*i)++;
	}
	char identifier = tFormatString[*i];

	char argumentText[100];
	char parsedValue[100];

	if (identifier == '%') {
		**tDst = '%';
		(*tDst)++;
		return;
	}
	else if(identifier == 'd' || identifier == 'i') {
		getArgumentTextAndAdvanceParams(argumentText, tParams);
		int val = atoi(argumentText);
		sprintf(parsedValue, "%d", val);
		int len = strlen(parsedValue);
		memcpy(*tDst, parsedValue, len);
		*tDst += len;
	}
	else if (identifier == 'f' || identifier == 'F') {
		getArgumentTextAndAdvanceParams(argumentText, tParams);
		double val = atof(argumentText);
		sprintf(parsedValue, "%f", val);
		int len = strlen(parsedValue);
		memcpy(*tDst, parsedValue, len);
		*tDst += len;
	}
	else if (identifier == 'e' || identifier == 'E') {
		getArgumentTextAndAdvanceParams(argumentText, tParams);
		double val = atof(argumentText);
		sprintf(parsedValue, "%e", val);
		int len = strlen(parsedValue);
		memcpy(*tDst, parsedValue, len);
		*tDst += len;
	}
	else if (identifier == 'g' || identifier == 'G') {
		getArgumentTextAndAdvanceParams(argumentText, tParams);
		double val = atof(argumentText);
		sprintf(parsedValue, "%g", val);
		int len = strlen(parsedValue);
		memcpy(*tDst, parsedValue, len);
		*tDst += len;
	}
	else {
		logWarning("Unrecognized parsing parameter");
		logWarningString(&tFormatString[*i - 1]);
		return;
	}



}

static void parseLinebreak(char** tDst, char* tTextBuffer) {
	**tDst = '\0';
	addClipboardLine(tTextBuffer);
	*tDst = tTextBuffer;
}

static void parseTab(char** tDst) {
	int j;
	for (j = 0; j < 4; j++) {
		**tDst = ' ';
		(*tDst)++;
	}
}

static void parseFormatInput(const char* tFormatString, int* i, char** tDst, char* tTextBuffer) {
	(*i)++;
	char identifier = tFormatString[*i];

	if (identifier == '\\') {
		**tDst = '\\';
		(*tDst)++;
	}
	else if (identifier == 't') {
		parseTab(tDst);
	}
	else if (identifier == 'n') {
		parseLinebreak(tDst, tTextBuffer);
	}
	else {
		logError("Unrecognized format parameter");
		logErrorString(&tFormatString[*i - 1]);
		recoverFromError();
	}
}

void addClipboardLineFormatString(const char * tFormatString, const char * tParameterString)
{
	char text[1024];
	char* dst;
	char paramBuffer[200];

	strcpy(paramBuffer, tParameterString);
	char* param = paramBuffer;

	dst = text;
	int len = strlen(tFormatString);
	int i;
	for (i = 0; i < len; i++) {
		if (tFormatString[i] == '%') {
			parseParameterInput(tFormatString, &i, &dst, &param);
		} else if(tFormatString[i] == '\\') {
			parseFormatInput(tFormatString, &i, &dst, text);
		}
		else if (tFormatString[i] == '\n') {
			parseLinebreak(&dst, text);
		}
		else if (tFormatString[i] == '\t') {
			parseTab(&dst);
		}
		else {
			*dst = tFormatString[i];
			dst++;
		}
	}

	*dst = '\0';
	if (dst != text) {
		addClipboardLine(text);
	}
}


void clipf(char* tFormatString, ...) {
	char text[1024];
	va_list args;
	va_start(args, tFormatString);
	vsprintf(text, tFormatString, args);
	va_end(args);

	addClipboardLine(text);
}

void clearClipboard()
{
	int i;
	for (i = 0; i < gPrismClipboardHandlerData.mLineAmount; i++) {
		gPrismClipboardHandlerData.mLines[i][0] = '\0';
	}
	setClipboardLineTexts();

	gPrismClipboardHandlerData.mLineAmount = 0;
}

void setClipboardInvisible()
{
	gPrismClipboardHandlerData.mIsVisible = 0;
	setClipboardLineTexts();
}

void setClipboardVisible()
{
	gPrismClipboardHandlerData.mIsVisible = 1;
	setClipboardLineTexts();
}
