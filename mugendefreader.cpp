#include "prism/mugendefreader.h"

#include <assert.h>
#include <ctype.h>
#include <algorithm>

#include "prism/file.h"
#include "prism/memoryhandler.h"
#include "prism/log.h"
#include "prism/system.h"
#include "prism/math.h"

using namespace std;

typedef struct MugenDefToken_t {
	char* mValue;
	struct MugenDefToken_t* mNext;

} MugenDefToken;

static struct {
	MugenDefToken* mRoot;
	MugenDefToken* mCurrent;
	MugenDefToken* mCurrentGroupToken;
	int mIsOver;
} gTokenReader;

static void setTokenReaderOver() {
	gTokenReader.mIsOver = 1;
}

static void addTokenToTokenReader(MugenDefToken* tToken) {
	if (!gTokenReader.mRoot) gTokenReader.mRoot = tToken;

	if (!gTokenReader.mCurrent) {
		gTokenReader.mCurrent = tToken;
	}
	else {
		gTokenReader.mCurrent->mNext = tToken;
		gTokenReader.mCurrent = tToken;
	}
}

static MugenDefToken* makeMugenDefToken(const char* tValue) {
	MugenDefToken* e = (MugenDefToken*)allocMemory(sizeof(MugenDefToken));
	e->mValue = (char*)allocMemory(strlen(tValue) + 10);
	strcpy(e->mValue, tValue);
	e->mNext = NULL;

	return e;
}

static int isEmpty(BufferPointer p) {
	return *p == ' ' || *p == '	' || *p < 0; // TODO: add symbol support for 128+
}

static int increaseAndCheckIfOver(Buffer* b, BufferPointer* p) {
	(*p)++;
	return ((uint32_t)*p == (uint32_t)b->mData + b->mLength);
}

static int decreaseAndCheckIfOver(Buffer* b, BufferPointer* p) {
	(*p)--;
	return (int32_t)(*p) < (int32_t)b->mData;
}

static int isComment(BufferPointer p) {
	return *p == ';';
}

static int isLinebreak(BufferPointer p) {
	return *p == '\n' || *p == 0xD || *p == 0xA;
}

static int increasePointerToNextLine(Buffer* b, BufferPointer* p) {
	while (!isLinebreak(*p)) {
		if (increaseAndCheckIfOver(b, p)) return 1;
	}

	while (isLinebreak(*p)) {
		if (increaseAndCheckIfOver(b, p)) return 1;
	}

	return 0;
}

static void parseComment(Buffer* b, BufferPointer* p) {
	
	if (increasePointerToNextLine(b, p)) {
		setTokenReaderOver();
	}
}

static BufferPointer findEndOfToken(Buffer* b, BufferPointer p, char start, char end, int tDoesCheckNesting) {
	assert(*p == start);

	int depth = 1;

	while ((depth > 0 || *p != end) && (uint32_t)p < ((uint32_t)b->mData)+b->mLength) {
		if (increaseAndCheckIfOver(b, &p)) {
			logError("Token reached end in wrong place."); // TODO: proper message
			logErrorString(p);
			recoverFromError();
		}
		if(tDoesCheckNesting) depth += *p == start;
		depth -= *p == end;
	}

	assert(*p == end);
	return p;
}

static BufferPointer getNextDefCharPosition(Buffer* b, BufferPointer p, char tChar) {
	while (*p != tChar) {
		increaseAndCheckIfOver(b, &p);
	}

	return p;
}

static char* makeMugenDefString(char* tPos, int tLength) {
	char* e = (char*)allocMemory(tLength + 10);
	memcpy(e, tPos, tLength);
	e[tLength] = '\0';
	return e;
}

static char* makeMugenDefStringFromEndPoint(BufferPointer tStart, BufferPointer tEnd) {
	int length = max(0, tEnd - tStart + 1);

	return makeMugenDefString(tStart, length);
}

static void destroyMugenDefString(char* tVal) {
	freeMemory(tVal);
}


static void moveBufferPointerForward(Buffer* b, BufferPointer* p) {
	while (isEmpty(*p) && !isLinebreak(*p)) {
		increaseAndCheckIfOver(b, p);
	}
}

static void moveBufferPointerBack(Buffer* b, BufferPointer* p) {
	while (isEmpty(*p)) {
		if(decreaseAndCheckIfOver(b, p)) {
			(*p)[100] = '\0';
			logError("Invalid parsing.");
			logErrorString(*p);
			recoverFromError();
		}
	}
}

static BufferPointer removeCommentFromToken(BufferPointer s, BufferPointer e) {
	BufferPointer i;
	for(i = s; i <= e; i++) {
		if (*i == ';') return i - 1;
	}

	return e;
}

static void parseAssignment(Buffer* b, BufferPointer* p, char tAssignmentToken) {
	debugLog("Parse assignment token.");

	BufferPointer equal = getNextDefCharPosition(b, *p, tAssignmentToken);
	MugenDefToken* equalToken = makeMugenDefToken("=");

	BufferPointer start = *p;
	BufferPointer end = equal - 1;
	moveBufferPointerForward(b, &start);
	moveBufferPointerBack(b, &end);
	char* text = makeMugenDefStringFromEndPoint(start, end);
	MugenDefToken* variableToken = makeMugenDefToken(text);
	destroyMugenDefString(text);

	start = equal + 1;
	end = equal;
	if (increasePointerToNextLine(b, &end)) end = ((char*)b->mData) + b->mLength - 1;
	else {
		decreaseAndCheckIfOver(b, &end);
	}
	while (isLinebreak(end)) decreaseAndCheckIfOver(b, &end);

	moveBufferPointerForward(b, &start);
	end = removeCommentFromToken(start, end);
	moveBufferPointerBack(b, &end);
	text = makeMugenDefStringFromEndPoint(start, end);
	MugenDefToken* valueToken = makeMugenDefToken(text);
	destroyMugenDefString(text);

	debugString(variableToken->mValue);
	debugString(valueToken->mValue);

	addTokenToTokenReader(equalToken);
	addTokenToTokenReader(variableToken);
	addTokenToTokenReader(valueToken);

	if (increasePointerToNextLine(b, p)) {
		setTokenReaderOver();
	}
}


char* getLineAsAllocatedString(Buffer* b, BufferPointer p) {
	BufferPointer start = p;
	BufferPointer end = p;
	if (increasePointerToNextLine(b, &end)) end = ((char*)b->mData) + b->mLength - 1;
	else {
		decreaseAndCheckIfOver(b, &end);
	}
	while (isLinebreak(end)) decreaseAndCheckIfOver(b, &end);
	end = removeCommentFromToken(start, end);
	moveBufferPointerBack(b, &end);
	char* text = makeMugenDefStringFromEndPoint(start, end);
	return text;
}

static void parseVectorStatement(Buffer* b, BufferPointer* p) {

	MugenDefToken* isVectorToken = makeMugenDefToken("vector_statement");

	char* text = getLineAsAllocatedString(b, *p);
	MugenDefToken* valueToken = makeMugenDefToken(text);
	destroyMugenDefString(text);

	addTokenToTokenReader(isVectorToken);
	addTokenToTokenReader(valueToken);

	if (increasePointerToNextLine(b, p)) {
		setTokenReaderOver();
	}
}

static void parseLoopStartStatement(Buffer* b, BufferPointer* p) {

	MugenDefToken* loopStartToken = makeMugenDefToken("Loopstart");
	addTokenToTokenReader(loopStartToken);

	if (increasePointerToNextLine(b, p)) {
		setTokenReaderOver();
	}
}

static void parseInterpolationStatement(Buffer* b, BufferPointer* p) {

	char* text = getLineAsAllocatedString(b, *p);
	MugenDefToken* interpolationToken = makeMugenDefToken(text);
	destroyMugenDefString(text);

	addTokenToTokenReader(interpolationToken);

	if (increasePointerToNextLine(b, p)) {
		setTokenReaderOver();
	}
}

static void parseTextStatement(Buffer* b, BufferPointer* p) {
	char* text = getLineAsAllocatedString(b, *p);
	MugenDefToken* textToken = makeMugenDefToken(text);
	destroyMugenDefString(text);
	addTokenToTokenReader(textToken);

	if (increasePointerToNextLine(b, p)) {
		setTokenReaderOver();
	}
}

static void parseGroup(Buffer* b, BufferPointer* p) {
	debugLog("Parse group.");
	BufferPointer end = findEndOfToken(b, *p, '[', ']', 0);

	char* val = makeMugenDefStringFromEndPoint(*p, end);
	MugenDefToken* groupToken = makeMugenDefToken(val);
	destroyMugenDefString(val);

	gTokenReader.mCurrentGroupToken = groupToken;
	addTokenToTokenReader(groupToken);

	debugString(groupToken->mValue);

	if (increasePointerToNextLine(b, p)) {
		setTokenReaderOver();
	}
}

static int isGroup(BufferPointer p) {
	return *p == '[';
}

static char* searchForChar(Buffer* b, BufferPointer p, char tChar) {
	while (p < (char*)b->mData + b->mLength && !isLinebreak(p) && !isComment(p)) {
		if (*p == tChar) return p;
		p++;
	}

	return NULL;
}

static int isAssignment(Buffer* b, BufferPointer p, char tAssignmentChar) {
	char* assignmentPos = searchForChar(b, p, tAssignmentChar);
	if (!assignmentPos) return 0;

	char* commaPos = searchForChar(b, p, ',');
	return !commaPos || (assignmentPos < commaPos);
}

static int isVectorStatement(Buffer* b, BufferPointer p) {
	return (int)searchForChar(b, p, ',');
}

static int isLoopStartStatement(Buffer* b, BufferPointer p) {
	char* text = getLineAsAllocatedString(b, p);
	turnStringLowercase(text);
	int ret = 0;
	ret |= !strcmp("loopstart", text);
	ret |= !strcmp("startloop", text); // TODO: check?

	destroyMugenDefString(text);
	return ret;
}

static int isInterpolationStatement(Buffer* b, BufferPointer p) {
	char* text = getLineAsAllocatedString(b, p);

	int ret = 0;
	ret |= !strcmp("Interpolate Offset", text);
	ret |= !strcmp("Interpolate Blend", text);
	ret |= !strcmp("Interpolate Scale", text);
	ret |= !strcmp("Interpolate Angle", text);

	destroyMugenDefString(text);
	return ret;
}

static int isTextStatement() {
	if (!gTokenReader.mCurrentGroupToken) return 0;
	char* text = gTokenReader.mCurrentGroupToken->mValue;

	int ret = 0;
	ret |= !strcmp("[Infobox Text]", text);
	ret |= !strcmp("[ja.Infobox Text]", text);
	ret |= !strcmp("[ExtraStages]", text); // TODO: check
	ret |= !strcmp("[Stories]", text); // TODO: check
	ret |= !strcmp("[Map]", text); // TODO: check
	ret |= !strcmp("[HitObjects]", text); 

	return ret;
}

static int isSpecialStatement(Buffer* b, BufferPointer p) {
	char* text = getLineAsAllocatedString(b, p);
	turnStringLowercase(text);
	int ret = 0;
	ret |= !strcmp("randomselect", text);

	destroyMugenDefString(text);
	return ret;
}

static void parseSingleToken(Buffer* b, BufferPointer* p) {
	while (isEmpty(*p) || isLinebreak(*p)) {
		if (increaseAndCheckIfOver(b, p)) {
			setTokenReaderOver();
			return;
		}
	}

	if (isComment(*p)) parseComment(b, p);
	else if (isGroup(*p)) parseGroup(b, p);
	else if (isTextStatement()) parseTextStatement(b, p);
	else if(isAssignment(b, *p, '=')) parseAssignment(b, p, '=');
	else if(isAssignment(b, *p, ':')) parseAssignment(b, p, ':');
	else if (isVectorStatement(b, *p)) parseVectorStatement(b, p);
	else if (isLoopStartStatement(b, *p)) parseLoopStartStatement(b, p);
	else if (isInterpolationStatement(b, *p)) parseInterpolationStatement(b, p);
	else if (isSpecialStatement(b, *p)) parseTextStatement(b, p);
	else {
		logWarningFormat("Unable to parse token:\n%.100s\n", (char*)(*p));
		parseComment(b, p);
		return;
	}
}

static void resetTokenReader() {
	gTokenReader.mRoot = NULL;
	gTokenReader.mCurrent = NULL;
	gTokenReader.mCurrentGroupToken = NULL;
	gTokenReader.mIsOver = 0;
}

static MugenDefToken* parseTokens(Buffer* b) {
	BufferPointer p = getBufferPointer(*b);

	resetTokenReader();
	while (!gTokenReader.mIsOver) {
		parseSingleToken(b, &p);
	}

	return gTokenReader.mRoot;
}

static MugenDefScript makeEmptyMugenDefScript() {
	MugenDefScript d;
	d.mGroups.clear();
	d.mFirstGroup = NULL;
	return d;
}

static int isGroupToken(MugenDefToken* t) {
	return t->mValue[0] == '[';
}

static struct {
	string mGroup;
	int mDoubleIndex;
} gScriptMaker;


static void setGroup(MugenDefScript* tScript, MugenDefToken* t) {
	debugLog("Setting group.");

	MugenDefScriptGroup* prev;
	if (stl_map_contains(tScript->mGroups, gScriptMaker.mGroup)) {
		prev = &tScript->mGroups[gScriptMaker.mGroup];
	}
	else {
		prev = NULL;
	}
		
	gScriptMaker.mGroup = string(t->mValue + 1, strlen(t->mValue + 1) - 1);

	MugenDefScriptGroup e;
	e.mElements.clear();
	e.mName = gScriptMaker.mGroup;
	e.mNext = NULL;
	e.mOrderedElementList = new_list();

	if (stl_map_contains(tScript->mGroups, gScriptMaker.mGroup)) {
		char temp[100];
		sprintf(temp, "%s %d", gScriptMaker.mGroup.data(), gScriptMaker.mDoubleIndex++);
		gScriptMaker.mGroup = string(temp);
	}

	assert(!stl_map_contains(tScript->mGroups, gScriptMaker.mGroup));
	tScript->mGroups[gScriptMaker.mGroup] = e;

	if (prev != NULL) {
		prev->mNext = &tScript->mGroups[gScriptMaker.mGroup];
	}

	if (tScript->mFirstGroup == NULL) {
		tScript->mFirstGroup = &tScript->mGroups[gScriptMaker.mGroup];
	}

	debugString(gScriptMaker.mGroup);
}

static int isStringToken(MugenDefToken* t) {
	return t->mValue[0] == '"';
}

static int isVectorToken(MugenDefToken* t) {
	int len = strlen(t->mValue);
	int i;
	int isVector = 0;
	for (i = 0; i < len; i++) {
		if (t->mValue[i] == ',') isVector = 1;
	}

	return isVector;
}


static int isNumberToken(MugenDefToken* t) {
	char* p;
	if (t->mValue[0] == '-') p = t->mValue + 1;
	else p = t->mValue;

	int len = strlen(p);
	int i;
	for (i = 0; i < len; i++) {
		if (*p < '0' || *p > '9') return 0;
		p++;
	}

	
	return 1;
}

static int isFloatToken(MugenDefToken* t) {
	char* p;
	if (t->mValue[0] == '-') p = t->mValue + 1;
	else p = t->mValue;

	int len = strlen(p);
	int i;
	int hasPoint = 0;
	for (i = 0; i < len; i++) {
		if (*p == '.') {
			if (hasPoint) return 0;
			else hasPoint = 1;
		}
		else if (*p < '0' || *p > '9') return 0;

		p++;
	}

	return 1;
}

static void setStringElement(MugenDefScriptGroupElement* element, MugenDefToken* t) {
	debugLog("Setting string element.");
	element->mType = MUGEN_DEF_SCRIPT_GROUP_STRING_ELEMENT;
	
	MugenDefScriptStringElement* e = (MugenDefScriptStringElement*)allocMemory(sizeof(MugenDefScriptStringElement));
	e->mString = (char*)allocMemory(strlen(t->mValue) + 10);
	strcpy(e->mString, t->mValue);
	element->mData = e;

	debugString(e->mString);
}

static void setNumberElement(MugenDefScriptGroupElement* element, MugenDefToken* t) {
	debugLog("Setting number element.");
	element->mType = MUGEN_DEF_SCRIPT_GROUP_NUMBER_ELEMENT;

	MugenDefScriptNumberElement* e = (MugenDefScriptNumberElement*)allocMemory(sizeof(MugenDefScriptNumberElement));
	e->mValue = atoi(t->mValue);
	element->mData = e;

	debugInteger(e->mValue);
}

static void setFloatElement(MugenDefScriptGroupElement* element, MugenDefToken* t) {
	debugLog("Setting float element.");
	element->mType = MUGEN_DEF_SCRIPT_GROUP_FLOAT_ELEMENT;

	MugenDefScriptFloatElement* e = (MugenDefScriptFloatElement*)allocMemory(sizeof(MugenDefScriptFloatElement));
	e->mValue = atof(t->mValue);
	element->mData = e;

	debugDouble(e->mValue);
}



static void setVectorElement(MugenDefScriptGroupElement* element, MugenDefToken* t) {
	debugLog("Setting vector element.");
	element->mType = MUGEN_DEF_SCRIPT_GROUP_VECTOR_ELEMENT;

	MugenDefScriptVectorElement* e = (MugenDefScriptVectorElement*)allocMemory(sizeof(MugenDefScriptVectorElement));
	
	char* comma = t->mValue - 1;
	e->mVector.mSize = 0;
	while (comma != NULL) {
		e->mVector.mSize++;
		comma = strchr(comma + 1, ',');
	}

	e->mVector.mElement = (char**)allocMemory(sizeof(char*)*e->mVector.mSize);
	
	comma = t->mValue - 1;
	int i = 0;
	while (comma != NULL) {
		char temp[MUGEN_DEF_STRING_LENGTH];
		assert(strlen(comma + 1) < MUGEN_DEF_STRING_LENGTH);
		strcpy(temp, comma+1);
		char* tempComma = strchr(temp, ',');
		if (tempComma != NULL) *tempComma = '\0';
		comma = strchr(comma + 1, ',');

		int offset = 0;
		while (temp[offset] == ' ') offset++;
		char* start = temp + offset;

		e->mVector.mElement[i] = (char*)allocMemory(strlen(start) + 3);
		strcpy(e->mVector.mElement[i], start);

		i++;
	}

	assert(i == e->mVector.mSize);

	element->mData = e;
}

static void setRawElement(MugenDefScriptGroupElement* element, MugenDefToken* t) {
	debugLog("Setting raw element.");
	element->mType = MUGEN_DEF_SCRIPT_GROUP_STRING_ELEMENT;

	MugenDefScriptStringElement* e = (MugenDefScriptStringElement*)allocMemory(sizeof(MugenDefScriptStringElement));
	e->mString = (char*)allocMemory(strlen(t->mValue) + 10);
	strcpy(e->mString, t->mValue);
	element->mData = e;

	debugString(e->mString);
}

static void unloadMugenDefElement(void* tCaller, void* tData);

static void addGroupElementToGroup(MugenDefScript* tScript, MugenDefScriptGroupElement tElement, char* tVariableName) {
	debugString(gScriptMaker.mGroup);
	if (!stl_map_contains(tScript->mGroups, gScriptMaker.mGroup)) {
		logWarning("Unable to add element to group, no group declared yet. Ignoring element.");
		unloadMugenDefElement(NULL, &tElement);
		return;
	}
	
	tElement.mName = string(tVariableName);
	MugenDefScriptGroup* e = &tScript->mGroups[gScriptMaker.mGroup];

	char variableName[100];
	if (stl_map_contains(e->mElements, tElement.mName)) {
		sprintf(variableName, "%s %d", tVariableName, gScriptMaker.mDoubleIndex++);
	}
	else {
		strcpy(variableName, tVariableName);
	}

	string variableNameString(variableName);
	assert(!stl_map_contains(e->mElements, variableNameString));
	
	e->mElements[variableNameString] = tElement;
	list_push_back(&e->mOrderedElementList, &e->mElements[variableNameString]);
}

static void setAssignment(MugenDefScript* tScript, MugenDefToken** tToken) {
	debugLog("Setting assignment.");

	assert(*tToken != NULL);
	assert(!strcmp("=", (*tToken)->mValue));
	*tToken = (*tToken)->mNext;

	MugenDefToken* variableToken = *tToken;
	*tToken = (*tToken)->mNext;
	assert(variableToken != NULL);
	debugString(variableToken->mValue);
	turnStringLowercase(variableToken->mValue);

	MugenDefToken* valueToken = *tToken;
	assert(valueToken != NULL);
	debugString(valueToken->mValue);

	MugenDefScriptGroupElement element;
	if (isStringToken(valueToken)) {
		setStringElement(&element, valueToken);
	}
	else if (isNumberToken(valueToken)) {
		setNumberElement(&element, valueToken);
	}
	else if (isFloatToken(valueToken)) {
		setFloatElement(&element, valueToken);
	}
	else if (isVectorToken(valueToken)) {
		setVectorElement(&element, valueToken);
	}
	else {
		setRawElement(&element, valueToken);
	}

	debugLog("Adding assignment to group.");
	addGroupElementToGroup(tScript, element, variableToken->mValue);
}

static int gVectorStatementCounter;

static void setVectorStatement(MugenDefScript* tScript, MugenDefToken** tToken) {
	debugLog("Setting vector statement.");

	assert(*tToken != NULL);
	assert(!strcmp("vector_statement", (*tToken)->mValue));
	*tToken = (*tToken)->mNext;

	MugenDefToken* valueToken = *tToken;
	assert(valueToken != NULL);
	debugString(valueToken->mValue);

	MugenDefScriptGroupElement element;
	assert(isVectorToken(*tToken));
	setVectorElement(&element, valueToken);
	
	debugLog("Adding vector statement to group.");
	char key[100];
	sprintf(key, "vector_statement %d", gVectorStatementCounter++);
	addGroupElementToGroup(tScript, element, key);
}

static int gLoopStartStatementCounter;

static void setLoopStartStatement(MugenDefScript* tScript, MugenDefToken** tToken) {
	debugLog("Setting loop start.");

	assert(*tToken != NULL);
	assert(!strcmp("Loopstart", (*tToken)->mValue));

	MugenDefScriptGroupElement element;
	setRawElement(&element, *tToken);

	debugLog("Adding loop start to group.");
	char key[100];
	sprintf(key, "Loopstart %d", gLoopStartStatementCounter++);
	addGroupElementToGroup(tScript, element, key);
}

static int gInterpolationStatementCounter;

static void setInterpolationStatement(MugenDefScript* tScript, MugenDefToken** tToken) {
	debugLog("Setting interpolation.");
	
	MugenDefToken* interpolationToken = *tToken;
	assert(interpolationToken != NULL);
	
	MugenDefScriptGroupElement element;
	setRawElement(&element, *tToken);

	debugLog("Adding interpolation to group.");
	char key[100];
	sprintf(key, "%s %d", interpolationToken->mValue, gInterpolationStatementCounter++);
	addGroupElementToGroup(tScript, element, key);
}

static void setTextStatement(MugenDefScript* tScript, MugenDefToken** tToken) {
	debugLog("Setting text.");

	assert(*tToken != NULL);

	MugenDefScriptGroupElement element;
	setRawElement(&element, *tToken);

	debugLog("Adding text to group.");
	char key[100];
	sprintf(key, "Text %d", gInterpolationStatementCounter++);
	addGroupElementToGroup(tScript, element, key);
}


static int isAssignmentToken(MugenDefToken* tToken) {
	return !strcmp(tToken->mValue, "=");
}

static int isVectorStatementToken(MugenDefToken* tToken) {
	return !strcmp(tToken->mValue, "vector_statement");
}

static int isLoopStartStatementToken(MugenDefToken* tToken) {
	return !strcmp(tToken->mValue, "Loopstart");
}

static int isInterpolationStatementToken(MugenDefToken* tToken) {
	
	if (!strcmp(tToken->mValue, "Interpolate Scale")) return 1;
	if (!strcmp(tToken->mValue, "Interpolate Angle")) return 1;
	if (!strcmp(tToken->mValue, "Interpolate Blend")) return 1;
	if (!strcmp(tToken->mValue, "Interpolate Offset")) return 1;

	return 0;
}

static int isTextStatementToken() {

	if (gScriptMaker.mGroup == "Infobox Text") return 1;
	if (gScriptMaker.mGroup == "ja.Infobox Text") return 1;
	if (gScriptMaker.mGroup == "ExtraStages") return 1;
	if (gScriptMaker.mGroup == "Stories") return 1;
	if (gScriptMaker.mGroup == "Map") return 1; // TODO: check
	if (gScriptMaker.mGroup == "HitObjects") return 1;

	return 0;
}


static void tokensToDefScript(MugenDefScript* tScript, MugenDefToken* tToken) {
	while(tToken) {
		MugenDefToken* startToken = tToken;
  
		if (isGroupToken(tToken)) {
			setGroup(tScript, tToken);
		}
		else if (isTextStatementToken()) {
			setTextStatement(tScript, &tToken);
		}
		else if(isAssignmentToken(tToken)){
			setAssignment(tScript, &tToken);
		}
		else if (isVectorStatementToken(tToken)) {
			setVectorStatement(tScript, &tToken);
		}
		else if (isLoopStartStatementToken(tToken)) {
			setLoopStartStatement(tScript, &tToken);
		}
		else if (isInterpolationStatementToken(tToken)) {
			setInterpolationStatement(tScript, &tToken);
		}
		else if (isInterpolationStatementToken(tToken)) {
			setInterpolationStatement(tScript, &tToken);
		}
		else {
			setTextStatement(tScript, &tToken);
		}

		MugenDefToken* finalToken = tToken->mNext;
		tToken = startToken; 
		while(tToken != finalToken) {
			MugenDefToken* next = tToken->mNext;
			freeMemory(tToken->mValue);
			freeMemory(tToken);
			tToken = next;
		}
	}
}

void loadMugenDefScript(MugenDefScript* oScript, string& tPath) { // TODO: refactor
	loadMugenDefScript(oScript, tPath.data());
}

void loadMugenDefScript(MugenDefScript* oScript, const char * tPath)
{
	debugLog("Start loading script.");
	debugString(tPath);
	logFormat("loading file %s\n", tPath);
	Buffer b = fileToBuffer(tPath);
	loadMugenDefScriptFromBufferAndFreeBuffer(oScript, b);
}

void loadMugenDefScriptFromBufferAndFreeBuffer(MugenDefScript* oScript, Buffer tBuffer) {
	MugenDefToken* root = parseTokens(&tBuffer);
	*oScript = makeEmptyMugenDefScript();
	freeBuffer(tBuffer);
	tokensToDefScript(oScript, root);
}

static void unloadMugenDefScriptFloatElement(MugenDefScriptFloatElement* e) {
	(void)e;
}

static void unloadMugenDefScriptNumberElement(MugenDefScriptNumberElement* e) {
	(void)e;
}

static void unloadMugenDefScriptVectorElement(MugenDefScriptVectorElement* e) {
	int i;
	for (i = 0; i < e->mVector.mSize; i++) {
		freeMemory(e->mVector.mElement[i]);
	}

	freeMemory(e->mVector.mElement);
}

static void unloadMugenDefScriptStringElement(MugenDefScriptStringElement* e) {
	freeMemory(e->mString);
}

static void unloadMugenDefElement(void* tCaller, void* tData) {
	(void)tCaller;
	MugenDefScriptGroupElement* e = (MugenDefScriptGroupElement*)tData;

	if (e->mType == MUGEN_DEF_SCRIPT_GROUP_FLOAT_ELEMENT) {
		unloadMugenDefScriptFloatElement((MugenDefScriptFloatElement*)e->mData);
	}
	else if (e->mType == MUGEN_DEF_SCRIPT_GROUP_NUMBER_ELEMENT) {
		unloadMugenDefScriptNumberElement((MugenDefScriptNumberElement*)e->mData);
	}
	else if (e->mType == MUGEN_DEF_SCRIPT_GROUP_VECTOR_ELEMENT) {
		unloadMugenDefScriptVectorElement((MugenDefScriptVectorElement*)e->mData);
	}
	else if (e->mType == MUGEN_DEF_SCRIPT_GROUP_STRING_ELEMENT) {
		unloadMugenDefScriptStringElement((MugenDefScriptStringElement*)e->mData);
	}
	else {
		logError("Unknown element type.");
		logErrorInteger(e->mType);
		recoverFromError();
	}

	freeMemory(e->mData);
}

static void unloadMugenDefScriptGroup(MugenDefScriptGroup* tGroup) {
	list_map(&tGroup->mOrderedElementList, unloadMugenDefElement, NULL);

	tGroup->mElements.clear();
	delete_list(&tGroup->mOrderedElementList);
}

void unloadMugenDefScript(MugenDefScript tScript)
{
	MugenDefScriptGroup* group = tScript.mFirstGroup;
	while (group != NULL) {
		MugenDefScriptGroup* next = group->mNext;
		unloadMugenDefScriptGroup(group);

		group = next;
	}

	tScript.mGroups.clear();
}

int isMugenDefStringVariable(MugenDefScript* tScript, const char * tGroupName, const char * tVariableName)
{
	if (!stl_string_map_contains_array(tScript->mGroups, tGroupName)) {
		return 0;
	}
	MugenDefScriptGroup* e = &tScript->mGroups[tGroupName];

	return isMugenDefStringVariableAsGroup(e, tVariableName);
}

char* getAllocatedMugenDefStringVariable(MugenDefScript* tScript, const char * tGroupName, const char * tVariableName)
{
	assert(stl_string_map_contains_array(tScript->mGroups, tGroupName));
	MugenDefScriptGroup* e = &tScript->mGroups[tGroupName];

	return 	getAllocatedMugenDefStringVariableAsGroup(e, tVariableName);
}

int isMugenDefStringVariableAsGroup(MugenDefScriptGroup* tGroup, const char* tVariableName) {
	if (!stl_string_map_contains_array(tGroup->mElements, tVariableName)) {
		return 0;
	}

	return 1;
}

char * getAllocatedMugenDefStringVariableAsGroup(MugenDefScriptGroup * tGroup, const char * tVariableName)
{
	assert(stl_string_map_contains_array(tGroup->mElements, tVariableName));
	MugenDefScriptGroupElement* element = &tGroup->mElements[tVariableName];

	return getAllocatedMugenDefStringVariableAsElement(element);
}

int isMugenDefStringVariableAsElement(MugenDefScriptGroupElement * tElement)
{
	(void)tElement;
	return 1;
}



static void turnMugenDefVectorToString(char* tDst, MugenDefScriptVectorElement* tVectorElement) {
	int i = 0;
	char* pos = tDst;
	*pos = '\0';
	for (i = 0; i < tVectorElement->mVector.mSize; i++) {
		if (i > 0) pos += sprintf(pos, ",");
		pos += sprintf(pos, "%s", tVectorElement->mVector.mElement[i]);
	}
}

static void checkStringElementForQuotesAndReturnRaw(char* tRet, MugenDefScriptStringElement* tStringElement) {
	int length = strlen(tStringElement->mString);
	if (length && tStringElement->mString[0] == '"' && tStringElement->mString[length - 1] == '"') {
		strcpy(tRet, tStringElement->mString + 1);
		int newLength = strlen(tRet);
		if (newLength) tRet[newLength - 1] = '\0';
	}
	else {
		strcpy(tRet, tStringElement->mString);
	}
}


char * getAllocatedMugenDefStringVariableAsElementGeneral(MugenDefScriptGroupElement * tElement, int tDoesReturnStringsRaw)
{
	char* ret = (char*)allocMemory(1024);
	if (tElement->mType == MUGEN_DEF_SCRIPT_GROUP_STRING_ELEMENT) {
		MugenDefScriptStringElement* stringElement = (MugenDefScriptStringElement*)tElement->mData;
		if (tDoesReturnStringsRaw) strcpy(ret, stringElement->mString);
		else checkStringElementForQuotesAndReturnRaw(ret, stringElement);

	}
	else if (tElement->mType == MUGEN_DEF_SCRIPT_GROUP_NUMBER_ELEMENT) {
		MugenDefScriptNumberElement* numberElement = (MugenDefScriptNumberElement*)tElement->mData;
		sprintf(ret, "%d", numberElement->mValue);
	}
	else if (tElement->mType == MUGEN_DEF_SCRIPT_GROUP_FLOAT_ELEMENT) {
		MugenDefScriptFloatElement* floatElement = (MugenDefScriptFloatElement*)tElement->mData;
		sprintf(ret, "%f", floatElement->mValue);
	}
	else if (tElement->mType == MUGEN_DEF_SCRIPT_GROUP_VECTOR_ELEMENT) {
		MugenDefScriptVectorElement* vectorElement = (MugenDefScriptVectorElement*)tElement->mData;
		turnMugenDefVectorToString(ret, vectorElement);
	}
	else {
		logError("Unknown type.");
		logErrorInteger(tElement->mType);
		recoverFromError();
	}
	return ret;
}

char * getAllocatedMugenDefStringVariableForAssignmentAsElement(MugenDefScriptGroupElement * tElement)
{
	return getAllocatedMugenDefStringVariableAsElementGeneral(tElement, 1);
}

char * getAllocatedMugenDefStringVariableAsElement(MugenDefScriptGroupElement * tElement)
{
	return getAllocatedMugenDefStringVariableAsElementGeneral(tElement, 0);
}

int isMugenDefVariable(MugenDefScript * tScript, const char * tGroupName, const char * tVariableName)
{
	return isMugenDefStringVariable(tScript, tGroupName, tVariableName);
}

int isMugenDefFloatVariable(MugenDefScript * tScript, const char * tGroupName, const char * tVariableName)
{
	if (!stl_string_map_contains_array(tScript->mGroups, tGroupName)) {
		return 0;
	}
	MugenDefScriptGroup* e = &tScript->mGroups[tGroupName];

	return isMugenDefFloatVariableAsGroup(e, tVariableName);
}

double getMugenDefFloatVariable(MugenDefScript * tScript, const char * tGroupName, const char * tVariableName)
{
	assert(stl_string_map_contains_array(tScript->mGroups, tGroupName));
	MugenDefScriptGroup* e = &tScript->mGroups[tGroupName];

	return getMugenDefFloatVariableAsGroup(e, tVariableName);
}

int isMugenDefFloatVariableAsGroup(MugenDefScriptGroup* tGroup, const char* tVariableName) {
	if (!stl_string_map_contains_array(tGroup->mElements, tVariableName)) {
		return 0;
	}

	MugenDefScriptGroupElement* element = &tGroup->mElements[tVariableName];

	return isMugenDefFloatVariableAsElement(element);
}

double getMugenDefFloatVariableAsGroup(MugenDefScriptGroup* tGroup, const char* tVariableName) {
	assert(stl_string_map_contains_array(tGroup->mElements, tVariableName));
	MugenDefScriptGroupElement* element = &tGroup->mElements[tVariableName];
	
	return getMugenDefFloatVariableAsElement(element);
}

int isMugenDefFloatVariableAsElement(MugenDefScriptGroupElement * tElement)
{
	return tElement->mType == MUGEN_DEF_SCRIPT_GROUP_FLOAT_ELEMENT || tElement->mType == MUGEN_DEF_SCRIPT_GROUP_NUMBER_ELEMENT;
}

double getMugenDefFloatVariableAsElement(MugenDefScriptGroupElement * tElement)
{
	assert(tElement->mType == MUGEN_DEF_SCRIPT_GROUP_FLOAT_ELEMENT || tElement->mType == MUGEN_DEF_SCRIPT_GROUP_NUMBER_ELEMENT);

	double ret;
	if (tElement->mType == MUGEN_DEF_SCRIPT_GROUP_NUMBER_ELEMENT) {
		MugenDefScriptNumberElement* numberElement = (MugenDefScriptNumberElement*)tElement->mData;
		ret = numberElement->mValue;
	}
	else if (tElement->mType == MUGEN_DEF_SCRIPT_GROUP_FLOAT_ELEMENT) {
		MugenDefScriptFloatElement* floatElement = (MugenDefScriptFloatElement*)tElement->mData;
		ret = floatElement->mValue;
	}
	else {
		ret = 0;
		logError("Unknown type.");
		logErrorInteger(tElement->mType);
		recoverFromError();
	}

	return ret;
}


int isMugenDefNumberVariable(MugenDefScript * tScript, const char * tGroupName, const char * tVariableName)
{
	if (!stl_string_map_contains_array(tScript->mGroups, tGroupName)) {
		return 0;
	}
	MugenDefScriptGroup* e = &tScript->mGroups[tGroupName];

	return isMugenDefNumberVariableAsGroup(e, tVariableName);
}


int getMugenDefNumberVariable(MugenDefScript * tScript, const char * tGroupName, const char * tVariableName)
{
	assert(stl_string_map_contains_array(tScript->mGroups, tGroupName));
	MugenDefScriptGroup* e = &tScript->mGroups[tGroupName];

	return getMugenDefNumberVariableAsGroup(e, tVariableName);
}

int isMugenDefNumberVariableAsGroup(MugenDefScriptGroup* tGroup, const char* tVariableName) {
	if (!stl_string_map_contains_array(tGroup->mElements, tVariableName)) {
		return 0;
	}

	MugenDefScriptGroupElement* element = &tGroup->mElements[tVariableName];

	return isMugenDefNumberVariableAsElement(element);
}

int getMugenDefNumberVariableAsGroup(MugenDefScriptGroup * tGroup, const char * tVariableName)
{
	assert(stl_string_map_contains_array(tGroup->mElements, tVariableName));
	MugenDefScriptGroupElement* element = &tGroup->mElements[tVariableName];

	return getMugenDefNumberVariableAsElement(element);
}

int isMugenDefNumberVariableAsElement(MugenDefScriptGroupElement * tElement)
{
	return tElement->mType == MUGEN_DEF_SCRIPT_GROUP_NUMBER_ELEMENT;
}

int getMugenDefNumberVariableAsElement(MugenDefScriptGroupElement * tElement)
{
	assert(tElement->mType == MUGEN_DEF_SCRIPT_GROUP_NUMBER_ELEMENT);

	int ret;
	if (tElement->mType == MUGEN_DEF_SCRIPT_GROUP_NUMBER_ELEMENT) {
		MugenDefScriptNumberElement* numberElement = (MugenDefScriptNumberElement*)tElement->mData;
		ret = numberElement->mValue;
	}
	else {
		ret = 0;
		logError("Unknown type.");
		logErrorInteger(tElement->mType);
		recoverFromError();
	}

	return ret;
}

int isMugenDefVectorVariable(MugenDefScript * tScript, const char * tGroupName, const char * tVariableName)
{
	if (!stl_string_map_contains_array(tScript->mGroups, tGroupName)) {
		return 0;
	}
	MugenDefScriptGroup* e = &tScript->mGroups[tGroupName];

	return isMugenDefVectorVariableAsGroup(e, tVariableName);
}

Vector3D getMugenDefVectorVariable(MugenDefScript * tScript, const char * tGroupName, const char * tVariableName)
{
	assert(stl_string_map_contains_array(tScript->mGroups, tGroupName));
	MugenDefScriptGroup* e = &tScript->mGroups[tGroupName];

	return getMugenDefVectorVariableAsGroup(e, tVariableName);
}

int isMugenDefVectorVariableAsGroup(MugenDefScriptGroup* tGroup, const char * tVariableName)
{
	if (!stl_string_map_contains_array(tGroup->mElements, tVariableName)) {
		return 0;
	}

	MugenDefScriptGroupElement* element = &tGroup->mElements[tVariableName];

	return isMugenDefVectorVariableAsElement(element);
}


Vector3D getMugenDefVectorVariableAsGroup(MugenDefScriptGroup* tGroup, const char* tVariableName) {
	assert(stl_string_map_contains_array(tGroup->mElements, tVariableName));
	MugenDefScriptGroupElement* element = &tGroup->mElements[tVariableName];

	return getMugenDefVectorVariableAsElement(element);
}

int isMugenDefVectorVariableAsElement(MugenDefScriptGroupElement * tElement)
{
	return tElement->mType == MUGEN_DEF_SCRIPT_GROUP_VECTOR_ELEMENT || tElement->mType == MUGEN_DEF_SCRIPT_GROUP_FLOAT_ELEMENT || tElement->mType == MUGEN_DEF_SCRIPT_GROUP_NUMBER_ELEMENT;
}

Vector3D getMugenDefVectorVariableAsElement(MugenDefScriptGroupElement * tElement)
{
	Vector3D ret;
	if (tElement->mType == MUGEN_DEF_SCRIPT_GROUP_VECTOR_ELEMENT) {
		MugenDefScriptVectorElement* vectorElement = (MugenDefScriptVectorElement*)tElement->mData;
		double x = atof(vectorElement->mVector.mElement[0]);
		double y = vectorElement->mVector.mSize >= 2 ? atof(vectorElement->mVector.mElement[1]) : 0;
		double z = vectorElement->mVector.mSize >= 3 ? atof(vectorElement->mVector.mElement[2]) : 0;
		ret = makePosition(x, y, z);
	} else if (tElement->mType == MUGEN_DEF_SCRIPT_GROUP_FLOAT_ELEMENT) {
		MugenDefScriptFloatElement* floatElement = (MugenDefScriptFloatElement*)tElement->mData;
		double x = floatElement->mValue;
		double y = 0;
		double z = 0;
		ret = makePosition(x, y, z);
	}
	else if (tElement->mType == MUGEN_DEF_SCRIPT_GROUP_NUMBER_ELEMENT) {
		MugenDefScriptNumberElement* numberElement = (MugenDefScriptNumberElement*)tElement->mData;
		double x = numberElement->mValue;
		double y = 0;
		double z = 0;
		ret = makePosition(x, y, z);
	}
	else {
		ret = makePosition(0, 0, 0);
		logError("Unknown type.");
		logErrorInteger(tElement->mType);
		recoverFromError();
	}

	return ret;
}

int isMugenDefVectorIVariable(MugenDefScript * tScript, const char * tGroupName, const char * tVariableName)
{
	if (!stl_string_map_contains_array(tScript->mGroups, tGroupName)) {
		return 0;
	}
	MugenDefScriptGroup* e = &tScript->mGroups[tGroupName];

	return isMugenDefVectorIVariableAsGroup(e, tVariableName);
}

int isMugenDefVectorIVariableAsGroup(MugenDefScriptGroup* tGroup, const char* tVariableName) {
	if (!stl_string_map_contains_array(tGroup->mElements, tVariableName)) {
		return 0;
	}

	MugenDefScriptGroupElement* element = &tGroup->mElements[tVariableName];

	return element->mType == MUGEN_DEF_SCRIPT_GROUP_VECTOR_ELEMENT || element->mType == MUGEN_DEF_SCRIPT_GROUP_NUMBER_ELEMENT;
}

Vector3DI getMugenDefVectorIVariableAsGroup(MugenDefScriptGroup* tGroup, const char* tVariableName) {
	assert(stl_string_map_contains_array(tGroup->mElements, tVariableName));
	MugenDefScriptGroupElement* element = &tGroup->mElements[tVariableName];

	Vector3DI ret;
	if (element->mType == MUGEN_DEF_SCRIPT_GROUP_VECTOR_ELEMENT) {
		MugenDefScriptVectorElement* vectorElement = (MugenDefScriptVectorElement*)element->mData;
		int x = atoi(vectorElement->mVector.mElement[0]);
		int y = vectorElement->mVector.mSize >= 2 ? atoi(vectorElement->mVector.mElement[1]) : 0;
		int z = vectorElement->mVector.mSize >= 3 ? atoi(vectorElement->mVector.mElement[2]) : 0;
		ret = makeVector3DI(x, y, z);
	}
	else if (element->mType == MUGEN_DEF_SCRIPT_GROUP_NUMBER_ELEMENT) {
		MugenDefScriptNumberElement* numberElement = (MugenDefScriptNumberElement*)element->mData;
		int x = numberElement->mValue;
		int y = 0;
		int z = 0;
		ret = makeVector3DI(x, y, z);
	}
	else {
		ret = makeVector3DI(0, 0, 0);
		logError("Unknown type.");
		logErrorInteger(element->mType);
		recoverFromError();
	}

	return ret;
}

Vector3DI getMugenDefVectorIVariable(MugenDefScript * tScript, const char * tGroupName, const char * tVariableName)
{
	assert(stl_string_map_contains_array(tScript->mGroups, tGroupName));
	MugenDefScriptGroup* e = &tScript->mGroups[tGroupName];

	return getMugenDefVectorIVariableAsGroup(e, tVariableName);
}

int isMugenDefStringVectorVariableAsGroup(MugenDefScriptGroup* tGroup, const char* tVariableName) {
	if (!stl_string_map_contains_array(tGroup->mElements, tVariableName)) {
		return 0;
	}

	MugenDefScriptGroupElement* element = &tGroup->mElements[tVariableName];
	return isMugenDefStringVectorVariableAsElement(element);
}

int isMugenDefStringVectorVariableAsElement(MugenDefScriptGroupElement * tElement)
{
	return tElement->mType == MUGEN_DEF_SCRIPT_GROUP_VECTOR_ELEMENT;
}

MugenStringVector getMugenDefStringVectorVariable(MugenDefScript* tScript, const char* tGroupName, const char* tVariableName) {
	assert(stl_string_map_contains_array(tScript->mGroups, tGroupName));
	MugenDefScriptGroup* e = &tScript->mGroups[tGroupName];

	return getMugenDefStringVectorVariableAsGroup(e, tVariableName);
}

MugenStringVector getMugenDefStringVectorVariableAsGroup(MugenDefScriptGroup* tGroup, const char* tVariableName) {
	assert(stl_string_map_contains_array(tGroup->mElements, tVariableName));
	MugenDefScriptGroupElement* element = &tGroup->mElements[tVariableName];

	return getMugenDefStringVectorVariableAsElement(element);
}

MugenStringVector getMugenDefStringVectorVariableAsElement(MugenDefScriptGroupElement * tElement)
{
	assert(tElement->mType == MUGEN_DEF_SCRIPT_GROUP_VECTOR_ELEMENT);
	MugenDefScriptVectorElement* vectorElement = (MugenDefScriptVectorElement*)tElement->mData;
	return vectorElement->mVector;
}

int isMugenDefGeoRectangleVariable(MugenDefScript* tScript, const char* tGroupName, const char* tVariableName) {
	if (!stl_string_map_contains_array(tScript->mGroups, tGroupName)) {
		return 0;
	}
	MugenDefScriptGroup* e = &tScript->mGroups[tGroupName];
	return isMugenDefGeoRectangleVariableAsGroup(e, tVariableName);
}

int isMugenDefGeoRectangleVariableAsGroup(MugenDefScriptGroup* tGroup, const char* tVariableName) {
	if (!stl_string_map_contains_array(tGroup->mElements, tVariableName)) {
		return 0;
	}
	MugenDefScriptGroupElement* element = &tGroup->mElements[tVariableName];
	return isMugenDefGeoRectangleVariableAsElement(element);
}

int isMugenDefGeoRectangleVariableAsElement(MugenDefScriptGroupElement * tElement) {
	if (tElement->mType != MUGEN_DEF_SCRIPT_GROUP_VECTOR_ELEMENT) return 0;

	MugenDefScriptVectorElement* vectorElement = (MugenDefScriptVectorElement*)tElement->mData;
	return vectorElement->mVector.mSize >= 4;
}

GeoRectangle getMugenDefGeoRectangleVariable(MugenDefScript* tScript, const char* tGroupName, const char* tVariableName) {
	assert(stl_string_map_contains_array(tScript->mGroups, tGroupName));
	MugenDefScriptGroup* e = &tScript->mGroups[tGroupName];
	return getMugenDefGeoRectangleVariableAsGroup(e, tVariableName);
}
GeoRectangle getMugenDefGeoRectangleVariableAsGroup(MugenDefScriptGroup* tGroup, const char* tVariableName) {
	assert(stl_string_map_contains_array(tGroup->mElements, tVariableName));
	MugenDefScriptGroupElement* element = &tGroup->mElements[tVariableName];
	return getMugenDefGeoRectangleVariableAsElement(element);
}
GeoRectangle getMugenDefGeoRectangleVariableAsElement(MugenDefScriptGroupElement * tElement) {
	assert(tElement->mType == MUGEN_DEF_SCRIPT_GROUP_VECTOR_ELEMENT);
	MugenDefScriptVectorElement* vectorElement = (MugenDefScriptVectorElement*)tElement->mData;
	assert(vectorElement->mVector.mSize >= 4);
	GeoRectangle ret;
	ret.mTopLeft.x = atof(vectorElement->mVector.mElement[0]);
	ret.mTopLeft.y = atof(vectorElement->mVector.mElement[1]);
	ret.mBottomRight.x = atof(vectorElement->mVector.mElement[2]);
	ret.mBottomRight.y = atof(vectorElement->mVector.mElement[3]);

	return ret;
}

MugenStringVector copyMugenDefStringVectorVariableAsElement(MugenDefScriptGroupElement * tElement)
{
	assert(tElement->mType == MUGEN_DEF_SCRIPT_GROUP_VECTOR_ELEMENT);
	MugenDefScriptVectorElement* vectorElement = (MugenDefScriptVectorElement*)tElement->mData;

	MugenDefScriptVectorElement ret;
	ret.mVector.mSize = vectorElement->mVector.mSize;
	ret.mVector.mElement = (char**)allocMemory(sizeof(char*)*ret.mVector.mSize);

	int i;
	for (i = 0; i < ret.mVector.mSize; i++) {
		ret.mVector.mElement[i] = (char*)allocMemory(strlen(vectorElement->mVector.mElement[i]) + 10);
		strcpy(ret.mVector.mElement[i], vectorElement->mVector.mElement[i]);
	}

	return ret.mVector;
}

void getMugenDefStringOrDefault(char* tDst, MugenDefScript* s, const char* tGroup, const char* tVariable, const char* tDefault) {
	if (isMugenDefStringVariable(s, tGroup, tVariable)) {
		char* res = getAllocatedMugenDefStringVariable(s, tGroup, tVariable);
		strcpy(tDst, res);
		freeMemory(res);
	}
	else {
		strcpy(tDst, tDefault);
	}
}

static char* createAllocatedString(const char* tString) {
	char* ret = (char*)allocMemory(strlen(tString) + 1);
	strcpy(ret, tString);
	return ret;
}

char* getAllocatedMugenDefStringOrDefault(MugenDefScript* tScript, const char* tGroupName, const char* tVariableName, const char* tDefault) {
	if (isMugenDefStringVariable(tScript, tGroupName, tVariableName)) return getAllocatedMugenDefStringVariable(tScript, tGroupName, tVariableName);
	else return createAllocatedString(tDefault);
}

char* getAllocatedMugenDefStringOrDefaultAsGroup(MugenDefScriptGroup* tGroup, const char* tVariable, const char* tDefault) {
	if (isMugenDefStringVariableAsGroup(tGroup, tVariable)) return getAllocatedMugenDefStringVariableAsGroup(tGroup, tVariable);
	else return createAllocatedString(tDefault);
}

double getMugenDefFloatOrDefault(MugenDefScript* s, const char* tGroup, const char* tVariable, double tDefault) {
	if (isMugenDefFloatVariable(s, tGroup, tVariable)) {
		return getMugenDefFloatVariable(s, tGroup, tVariable);
	}
	else {
		return tDefault;
	}
}

double getMugenDefFloatOrDefaultAsGroup(MugenDefScriptGroup* tGroup, const char* tVariable, double tDefault) {
	if (isMugenDefFloatVariableAsGroup(tGroup, tVariable)) {
		return getMugenDefFloatVariableAsGroup(tGroup, tVariable);
	}
	else {
		return tDefault;
	}
}

int getMugenDefIntegerOrDefault(MugenDefScript* s, const char* tGroup, const char* tVariable, int tDefault) {
	if (isMugenDefNumberVariable(s, tGroup, tVariable)) {
		return getMugenDefNumberVariable(s, tGroup, tVariable);
	}
	else {
		return tDefault;
	}
}

int getMugenDefIntegerOrDefaultAsGroup(MugenDefScriptGroup* tGroup, const char* tVariable, int tDefault) {
	if (isMugenDefNumberVariableAsGroup(tGroup, tVariable)) {
		return getMugenDefNumberVariableAsGroup(tGroup, tVariable);
	}
	else {
		return tDefault;
	}
}

Vector3D getMugenDefVectorOrDefault(MugenDefScript* s, const char* tGroup, const char* tVariable, Vector3D tDefault) {
	if (isMugenDefVectorVariable(s, tGroup, tVariable)) {
		return getMugenDefVectorVariable(s, tGroup, tVariable);
	}
	else {
		return tDefault;
	}
}

Vector3D getMugenDefVectorOrDefaultAsGroup(MugenDefScriptGroup* tGroup, const char* tVariable, Vector3D tDefault) {
	if (isMugenDefVectorVariableAsGroup(tGroup, tVariable)) {
		return getMugenDefVectorVariableAsGroup(tGroup, tVariable);
	}
	else {
		return tDefault;
	}
}

Vector3DI getMugenDefVectorIOrDefault(MugenDefScript* s, const char* tGroup, const char* tVariable, Vector3DI tDefault) {
	if (isMugenDefVectorIVariable(s, tGroup, tVariable)) {
		return getMugenDefVectorIVariable(s, tGroup, tVariable);
	}
	else {
		return tDefault;
	}
}


Vector3DI getMugenDefVectorIOrDefaultAsGroup(MugenDefScriptGroup* tGroup, const char* tVariable, Vector3DI tDefault) {
	if (isMugenDefVectorIVariableAsGroup(tGroup, tVariable)) {
		return getMugenDefVectorIVariableAsGroup(tGroup, tVariable);
	}
	else {
		return tDefault;
	}
}



GeoRectangle getMugenDefGeoRectangleOrDefault(MugenDefScript* s, const char* tGroup, const char* tVariable, GeoRectangle tDefault) {
	if (isMugenDefGeoRectangleVariable(s, tGroup, tVariable)) {
		return getMugenDefGeoRectangleVariable(s, tGroup, tVariable);
	}
	else {
		return tDefault;
	}
}

GeoRectangle getMugenDefGeoRectangleOrDefaultAsGroup(MugenDefScriptGroup* tGroup, const char* tVariable, GeoRectangle tDefault) {
	if (isMugenDefGeoRectangleVariableAsGroup(tGroup, tVariable)) {
		return getMugenDefGeoRectangleVariableAsGroup(tGroup, tVariable);
	}
	else {
		return tDefault;
	}
}
