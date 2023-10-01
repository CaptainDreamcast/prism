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
	e->mValue = (char*)allocMemory(int(strlen(tValue) + 10));
	strcpy(e->mValue, tValue);
	e->mNext = NULL;

	return e;
}

static int isEmpty(BufferPointer p) {
	return *p == ' ' || *p == '	'; //|| *p < 0; TODO: check if this breaks Dolmexica (PBI 3074)
}

static int increaseAndCheckIfOver(Buffer* b, BufferPointer* p) {
	(*p)++;
	return ((uintptr_t)*p == (uintptr_t)b->mData + b->mLength);
}

static int decreaseAndCheckIfOver(Buffer* b, BufferPointer* p) {
	(*p)--;
	return (uintptr_t)(*p) < (uintptr_t)b->mData;
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

	while ((depth > 0 || *p != end) && (uintptr_t)p < ((uintptr_t)b->mData)+b->mLength) {
		if (increaseAndCheckIfOver(b, &p)) {
			logError("Token reached end in wrong place.");
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
	int length = max(0, (int)(tEnd - tStart + 1));

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
	verboseLog("Parse assignment token.");

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

	verboseString(variableToken->mValue);
	verboseString(valueToken->mValue);

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
	turnStringLowercase(text);
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
	verboseLog("Parse group.");
	BufferPointer end = findEndOfToken(b, *p, '[', ']', 0);

	char* val = makeMugenDefStringFromEndPoint(*p, end);
	MugenDefToken* groupToken = makeMugenDefToken(val);
	destroyMugenDefString(val);
	turnStringLowercase(groupToken->mValue);

	gTokenReader.mCurrentGroupToken = groupToken;
	addTokenToTokenReader(groupToken);

	verboseString(groupToken->mValue);

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
	return searchForChar(b, p, ',') != NULL;
}

static int isLoopStartStatement(Buffer* b, BufferPointer p) {
	char* text = getLineAsAllocatedString(b, p);
	turnStringLowercase(text);
	int ret = 0;
	ret |= !strcmp("loopstart", text);
	ret |= !strcmp("startloop", text);

	destroyMugenDefString(text);
	return ret;
}

static int isInterpolationStatement(Buffer* b, BufferPointer p) {
	char* text = getLineAsAllocatedString(b, p);
	turnStringLowercase(text);

	int ret = 0;
	ret |= !strcmp("interpolate offset", text);
	ret |= !strcmp("interpolate blend", text);
	ret |= !strcmp("interpolate scale", text);
	ret |= !strcmp("interpolate angle", text);

	destroyMugenDefString(text);
	return ret;
}

static int isGroupOverridingTextStatement() {
	if (!gTokenReader.mCurrentGroupToken) return 0;
	const auto text = gTokenReader.mCurrentGroupToken->mValue;

	int ret = 0;
	ret |= !strcmp("[map]", text);
	return ret;
}

static int isTextStatement() {
	if (!gTokenReader.mCurrentGroupToken) return 0;
	const auto text = gTokenReader.mCurrentGroupToken->mValue;

	int ret = 0;
	ret |= !strcmp("[infobox text]", text);
	ret |= !strcmp("[ja.infobox text]", text);
	ret |= !strcmp("[extrastages]", text);
	ret |= !strcmp("[stories]", text);
	ret |= !strcmp("[hitobjects]", text); 

	return ret;
}

static int isPotentialTextStatement() {
	if (!gTokenReader.mCurrentGroupToken) return 0;
	const auto text = gTokenReader.mCurrentGroupToken->mValue;

	int ret = 0;
	ret |= !strcmp("[characters]", text);

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
	else if (isGroupOverridingTextStatement()) parseTextStatement(b, p);
	else if (isGroup(*p)) parseGroup(b, p);
	else if (isTextStatement()) parseTextStatement(b, p);
	else if(isAssignment(b, *p, '=')) parseAssignment(b, p, '=');
	else if(isAssignment(b, *p, ':')) parseAssignment(b, p, ':');
	else if (isVectorStatement(b, *p)) parseVectorStatement(b, p);
	else if (isLoopStartStatement(b, *p)) parseLoopStartStatement(b, p);
	else if (isInterpolationStatement(b, *p)) parseInterpolationStatement(b, p);
	else if (isPotentialTextStatement()) parseTextStatement(b, p);
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
	verboseLog("Setting group.");

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

	verboseString(gScriptMaker.mGroup);
}

static int isStringToken(MugenDefToken* t) {
	return t->mValue[0] == '"';
}

static int isVectorToken(MugenDefToken* t) {
	auto len = int(strlen(t->mValue));
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

	auto len = int(strlen(p));
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

	auto len = int(strlen(p));
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
	verboseLog("Setting string element.");
	element->mType = MUGEN_DEF_SCRIPT_GROUP_STRING_ELEMENT;
	
	MugenDefScriptStringElement* e = (MugenDefScriptStringElement*)allocMemory(sizeof(MugenDefScriptStringElement));
	e->mString = (char*)allocMemory(int(strlen(t->mValue) + 10));
	strcpy(e->mString, t->mValue);
	element->mData = e;

	verboseString(e->mString);
}

static void setNumberElement(MugenDefScriptGroupElement* element, MugenDefToken* t) {
	verboseLog("Setting number element.");
	element->mType = MUGEN_DEF_SCRIPT_GROUP_NUMBER_ELEMENT;

	MugenDefScriptNumberElement* e = (MugenDefScriptNumberElement*)allocMemory(sizeof(MugenDefScriptNumberElement));
	e->mValue = atoi(t->mValue);
	element->mData = e;

	verboseInteger(e->mValue);
}

static void setFloatElement(MugenDefScriptGroupElement* element, MugenDefToken* t) {
	verboseLog("Setting float element.");
	element->mType = MUGEN_DEF_SCRIPT_GROUP_FLOAT_ELEMENT;

	MugenDefScriptFloatElement* e = (MugenDefScriptFloatElement*)allocMemory(sizeof(MugenDefScriptFloatElement));
	e->mValue = atof(t->mValue);
	element->mData = e;

	verboseDouble(e->mValue);
}



static void setVectorElement(MugenDefScriptGroupElement* element, MugenDefToken* t) {
	verboseLog("Setting vector element.");
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

		e->mVector.mElement[i] = (char*)allocMemory(int(strlen(start) + 3));
		strcpy(e->mVector.mElement[i], start);

		i++;
	}

	assert(i == e->mVector.mSize);

	element->mData = e;
}

static void setRawElement(MugenDefScriptGroupElement* element, MugenDefToken* t) {
	verboseLog("Setting raw element.");
	element->mType = MUGEN_DEF_SCRIPT_GROUP_STRING_ELEMENT;

	MugenDefScriptStringElement* e = (MugenDefScriptStringElement*)allocMemory(sizeof(MugenDefScriptStringElement));
	e->mString = (char*)allocMemory(int(strlen(t->mValue) + 10));
	strcpy(e->mString, t->mValue);
	element->mData = e;

	verboseString(e->mString);
}

static void unloadMugenDefElement(void* tCaller, void* tData);

static void addGroupElementToGroup(MugenDefScript* tScript, MugenDefScriptGroupElement tElement, char* tVariableName) {
	verboseString(gScriptMaker.mGroup);
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
	verboseLog("Setting assignment.");

	assert(*tToken != NULL);
	assert(!strcmp("=", (*tToken)->mValue));
	*tToken = (*tToken)->mNext;

	MugenDefToken* variableToken = *tToken;
	*tToken = (*tToken)->mNext;
	assert(variableToken != NULL);
	verboseString(variableToken->mValue);
	turnStringLowercase(variableToken->mValue);

	MugenDefToken* valueToken = *tToken;
	assert(valueToken != NULL);
	verboseString(valueToken->mValue);

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

	verboseLog("Adding assignment to group.");
	addGroupElementToGroup(tScript, element, variableToken->mValue);
}

static int gVectorStatementCounter;

static void setVectorStatement(MugenDefScript* tScript, MugenDefToken** tToken) {
	verboseLog("Setting vector statement.");

	assert(*tToken != NULL);
	assert(!strcmp("vector_statement", (*tToken)->mValue));
	*tToken = (*tToken)->mNext;

	MugenDefToken* valueToken = *tToken;
	assert(valueToken != NULL);
	verboseString(valueToken->mValue);

	MugenDefScriptGroupElement element;
	assert(isVectorToken(*tToken));
	setVectorElement(&element, valueToken);
	
	verboseLog("Adding vector statement to group.");
	char key[100];
	sprintf(key, "vector_statement %d", gVectorStatementCounter++);
	addGroupElementToGroup(tScript, element, key);
}

static int gLoopStartStatementCounter;

static void setLoopStartStatement(MugenDefScript* tScript, MugenDefToken** tToken) {
	verboseLog("Setting loop start.");

	assert(*tToken != NULL);
	assert(!strcmp("Loopstart", (*tToken)->mValue));

	MugenDefScriptGroupElement element;
	setRawElement(&element, *tToken);

	verboseLog("Adding loop start to group.");
	char key[100];
	sprintf(key, "Loopstart %d", gLoopStartStatementCounter++);
	addGroupElementToGroup(tScript, element, key);
}

static int gInterpolationStatementCounter;

static void setInterpolationStatement(MugenDefScript* tScript, MugenDefToken** tToken) {
	verboseLog("Setting interpolation.");
	
	MugenDefToken* interpolationToken = *tToken;
	assert(interpolationToken != NULL);
	
	MugenDefScriptGroupElement element;
	setRawElement(&element, *tToken);

	verboseLog("Adding interpolation to group.");
	char key[100];
	sprintf(key, "%s %d", interpolationToken->mValue, gInterpolationStatementCounter++);
	addGroupElementToGroup(tScript, element, key);
}

static void setTextStatement(MugenDefScript* tScript, MugenDefToken** tToken) {
	verboseLog("Setting text.");

	assert(*tToken != NULL);

	MugenDefScriptGroupElement element;
	setRawElement(&element, *tToken);

	verboseLog("Adding text to group.");
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
	
	if (!strcmp(tToken->mValue, "interpolate scale")) return 1;
	if (!strcmp(tToken->mValue, "interpolate angle")) return 1;
	if (!strcmp(tToken->mValue, "interpolate blend")) return 1;
	if (!strcmp(tToken->mValue, "interpolate offset")) return 1;

	return 0;
}

static int isTextStatementToken() {

	if (gScriptMaker.mGroup == "infobox text") return 1;
	if (gScriptMaker.mGroup == "ja.infobox text") return 1;
	if (gScriptMaker.mGroup == "extrastages") return 1;
	if (gScriptMaker.mGroup == "stories") return 1;
	if (gScriptMaker.mGroup == "map") return 1;
	if (gScriptMaker.mGroup == "hitobjects") return 1;

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

void loadMugenDefScript(MugenDefScript* oScript, const string& tPath) {
	loadMugenDefScript(oScript, tPath.c_str());
}

void loadMugenDefScript(MugenDefScript* oScript, const char * tPath)
{
	debugLog("Start loading script.");
	debugString(tPath);
	Buffer b = fileToBuffer(tPath);
	loadMugenDefScriptFromBufferAndFreeBuffer(oScript, b);
}

void loadMugenDefScriptFromBufferAndFreeBuffer(MugenDefScript* oScript, Buffer& tBuffer) {
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
	destroyMugenStringVector(e->mVector);
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

void unloadMugenDefScript(MugenDefScript* tScript)
{
	MugenDefScriptGroup* group = tScript->mFirstGroup;
	while (group != NULL) {
		MugenDefScriptGroup* next = group->mNext;
		unloadMugenDefScriptGroup(group);

		group = next;
	}

	tScript->mGroups.clear();
}

int hasMugenDefScriptGroup(MugenDefScript* tScript, const char* tGroupName) {
	assert(isStringLowercase(tGroupName));
	return stl_string_map_contains_array(tScript->mGroups, tGroupName);
}

MugenDefScriptGroup* getMugenDefScriptGroup(MugenDefScript* tScript, const char* tGroupName) {
	assert(isStringLowercase(tGroupName));
	assert(stl_string_map_contains_array(tScript->mGroups, tGroupName));
	return &tScript->mGroups[tGroupName];
}

MugenStringVector createAllocatedMugenStringVectorFromString(const char* tString)
{
	MugenStringVector ret;
	ret.mSize = 1;
	ret.mElement = (char**)allocMemory(sizeof(char*));
	ret.mElement[0] = (char*)allocMemory(int(strlen(tString) + 3));
	strcpy(ret.mElement[0], tString);
	return ret;
}

void destroyMugenStringVector(MugenStringVector& tStringVector)
{
	for (int i = 0; i < tStringVector.mSize; i++) {
		freeMemory(tStringVector.mElement[i]);
	}

	freeMemory(tStringVector.mElement);
}

int isMugenDefStringVariable(MugenDefScript* tScript, const char * tGroupName, const char * tVariableName)
{
	assert(isStringLowercase(tGroupName));
	assert(isStringLowercase(tVariableName));
	if (!stl_string_map_contains_array(tScript->mGroups, tGroupName)) {
		return 0;
	}
	MugenDefScriptGroup* e = &tScript->mGroups[tGroupName];

	return isMugenDefStringVariableAsGroup(e, tVariableName);
}

char* getAllocatedMugenDefStringVariable(MugenDefScript* tScript, const char * tGroupName, const char * tVariableName)
{
	assert(isStringLowercase(tGroupName));
	assert(isStringLowercase(tVariableName));
	assert(stl_string_map_contains_array(tScript->mGroups, tGroupName));
	MugenDefScriptGroup* e = &tScript->mGroups[tGroupName];

	return 	getAllocatedMugenDefStringVariableAsGroup(e, tVariableName);
}

string getSTLMugenDefStringVariable(MugenDefScript * tScript, const char * tGroupName, const char * tVariableName)
{
	assert(isStringLowercase(tGroupName));
	assert(isStringLowercase(tVariableName));
	assert(stl_string_map_contains_array(tScript->mGroups, tGroupName));
	MugenDefScriptGroup* e = &tScript->mGroups[tGroupName];

	return 	getSTLMugenDefStringVariableAsGroup(e, tVariableName);
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

string getSTLMugenDefStringVariableAsGroup(MugenDefScriptGroup * tGroup, const char * tVariableName)
{
	assert(stl_string_map_contains_array(tGroup->mElements, tVariableName));
	MugenDefScriptGroupElement* element = &tGroup->mElements[tVariableName];

	return getSTLMugenDefStringVariableAsElement(element);
}

char* getAllocatedMugenDefStringVariableAsGroupForceAddWhiteSpaces(MugenDefScriptGroup* tGroup, const char* tVariableName)
{
	assert(stl_string_map_contains_array(tGroup->mElements, tVariableName));
	MugenDefScriptGroupElement* element = &tGroup->mElements[tVariableName];

	return getAllocatedMugenDefStringVariableAsElementForceAddWhiteSpaces(element);
}

std::string getSTLMugenDefStringVariableAsGroupForceAddWhiteSpaces(MugenDefScriptGroup* tGroup, const char* tVariableName)
{
	assert(stl_string_map_contains_array(tGroup->mElements, tVariableName));
	MugenDefScriptGroupElement* element = &tGroup->mElements[tVariableName];

	return getSTLMugenDefStringVariableAsElementForceAddWhiteSpaces(element);
}

int isMugenDefStringVariableAsElement(MugenDefScriptGroupElement * tElement)
{
	(void)tElement;
	return 1;
}



static void turnMugenDefVectorToString(char* tDst, MugenDefScriptVectorElement* tVectorElement, bool tDoesAddWhiteSpaces) {
	int i = 0;
	char* pos = tDst;
	*pos = '\0';
	for (i = 0; i < tVectorElement->mVector.mSize; i++) {
		if (i > 0)
		{
			pos += sprintf(pos, ",");
			if (tDoesAddWhiteSpaces) pos += sprintf(pos, " ");
		}
		pos += sprintf(pos, "%s", tVectorElement->mVector.mElement[i]);
	}
}

static void checkStringElementForQuotesAndReturnRaw(char* tRet, MugenDefScriptStringElement* tStringElement) {
	int length = int(strlen(tStringElement->mString));
	if (length && tStringElement->mString[0] == '"' && tStringElement->mString[length - 1] == '"') {
		strcpy(tRet, tStringElement->mString + 1);
		int newLength = int(strlen(tRet));
		if (newLength) tRet[newLength - 1] = '\0';
	}
	else {
		strcpy(tRet, tStringElement->mString);
	}
}
static void getMugenDefStringVariableAsElementGeneral(char* tDst, MugenDefScriptGroupElement * tElement, int tDoesReturnStringsRaw, bool tDoesAddWhiteSpaces)
{
	if (tElement->mType == MUGEN_DEF_SCRIPT_GROUP_STRING_ELEMENT) {
		MugenDefScriptStringElement* stringElement = (MugenDefScriptStringElement*)tElement->mData;
		if (tDoesReturnStringsRaw) strcpy(tDst, stringElement->mString);
		else checkStringElementForQuotesAndReturnRaw(tDst, stringElement);

	}
	else if (tElement->mType == MUGEN_DEF_SCRIPT_GROUP_NUMBER_ELEMENT) {
		MugenDefScriptNumberElement* numberElement = (MugenDefScriptNumberElement*)tElement->mData;
		sprintf(tDst, "%d", numberElement->mValue);
	}
	else if (tElement->mType == MUGEN_DEF_SCRIPT_GROUP_FLOAT_ELEMENT) {
		MugenDefScriptFloatElement* floatElement = (MugenDefScriptFloatElement*)tElement->mData;
		sprintf(tDst, "%f", floatElement->mValue);
	}
	else if (tElement->mType == MUGEN_DEF_SCRIPT_GROUP_VECTOR_ELEMENT) {
		MugenDefScriptVectorElement* vectorElement = (MugenDefScriptVectorElement*)tElement->mData;
		turnMugenDefVectorToString(tDst, vectorElement, tDoesAddWhiteSpaces);
	}
	else {
		logError("Unknown type.");
		logErrorInteger(tElement->mType);
		recoverFromError();
	}
}

char * getAllocatedMugenDefStringVariableAsElementGeneral(MugenDefScriptGroupElement * tElement, int tDoesReturnStringsRaw, bool tDoesAddWhiteSpaces)
{
	char* ret = (char*)allocMemory(1024);
	getMugenDefStringVariableAsElementGeneral(ret, tElement, tDoesReturnStringsRaw, tDoesAddWhiteSpaces);
	return ret;
}

std::string getSTLMugenDefStringVariableAsElementGeneral(MugenDefScriptGroupElement * tElement, int tDoesReturnStringsRaw, bool tDoesAddWhiteSpaces)
{
	char ret[1024];
	getMugenDefStringVariableAsElementGeneral(ret, tElement, tDoesReturnStringsRaw, tDoesAddWhiteSpaces);
	return std::string(ret);
}

char * getAllocatedMugenDefStringVariableForAssignmentAsElement(MugenDefScriptGroupElement * tElement)
{
	return getAllocatedMugenDefStringVariableAsElementGeneral(tElement, 1, false);
}

std::string getSTLMugenDefStringVariableForAssignmentAsElement(MugenDefScriptGroupElement * tElement)
{
	return getSTLMugenDefStringVariableAsElementGeneral(tElement, 1, false);
}

char* getAllocatedMugenDefStringVariableForAssignmentAsElementForceAddWhiteSpaces(MugenDefScriptGroupElement* tElement)
{
	return getAllocatedMugenDefStringVariableAsElementGeneral(tElement, 1, true);
}

std::string getSTLMugenDefStringVariableForAssignmentAsElementForceAddWhiteSpaces(MugenDefScriptGroupElement* tElement)
{
	return getSTLMugenDefStringVariableAsElementGeneral(tElement, 1, true);
}

char * getAllocatedMugenDefStringVariableAsElement(MugenDefScriptGroupElement * tElement)
{
	return getAllocatedMugenDefStringVariableAsElementGeneral(tElement, 0, false);
}

std::string getSTLMugenDefStringVariableAsElement(MugenDefScriptGroupElement * tElement)
{
	return getSTLMugenDefStringVariableAsElementGeneral(tElement, 0, false);
}

char* getAllocatedMugenDefStringVariableAsElementForceAddWhiteSpaces(MugenDefScriptGroupElement* tElement)
{
	return getAllocatedMugenDefStringVariableAsElementGeneral(tElement, 0, true);
}

std::string getSTLMugenDefStringVariableAsElementForceAddWhiteSpaces(MugenDefScriptGroupElement* tElement)
{
	return getSTLMugenDefStringVariableAsElementGeneral(tElement, 0, true);
}

int isMugenDefVariable(MugenDefScript * tScript, const char * tGroupName, const char * tVariableName)
{
	assert(isStringLowercase(tGroupName));
	assert(isStringLowercase(tVariableName));
	return isMugenDefStringVariable(tScript, tGroupName, tVariableName);
}

int isMugenDefFloatVariable(MugenDefScript * tScript, const char * tGroupName, const char * tVariableName)
{
	assert(isStringLowercase(tGroupName));
	assert(isStringLowercase(tVariableName));
	if (!stl_string_map_contains_array(tScript->mGroups, tGroupName)) {
		return 0;
	}
	MugenDefScriptGroup* e = &tScript->mGroups[tGroupName];

	return isMugenDefFloatVariableAsGroup(e, tVariableName);
}

double getMugenDefFloatVariable(MugenDefScript * tScript, const char * tGroupName, const char * tVariableName)
{
	assert(isStringLowercase(tGroupName));
	assert(isStringLowercase(tVariableName));
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
	return tElement->mType == MUGEN_DEF_SCRIPT_GROUP_FLOAT_ELEMENT || tElement->mType == MUGEN_DEF_SCRIPT_GROUP_NUMBER_ELEMENT || tElement->mType == MUGEN_DEF_SCRIPT_GROUP_VECTOR_ELEMENT;
}

double getMugenDefFloatVariableAsElement(MugenDefScriptGroupElement * tElement)
{
	assert(tElement->mType == MUGEN_DEF_SCRIPT_GROUP_FLOAT_ELEMENT || tElement->mType == MUGEN_DEF_SCRIPT_GROUP_NUMBER_ELEMENT || tElement->mType == MUGEN_DEF_SCRIPT_GROUP_VECTOR_ELEMENT);

	double ret;
	if (tElement->mType == MUGEN_DEF_SCRIPT_GROUP_NUMBER_ELEMENT) {
		MugenDefScriptNumberElement* numberElement = (MugenDefScriptNumberElement*)tElement->mData;
		ret = numberElement->mValue;
	}
	else if (tElement->mType == MUGEN_DEF_SCRIPT_GROUP_FLOAT_ELEMENT) {
		MugenDefScriptFloatElement* floatElement = (MugenDefScriptFloatElement*)tElement->mData;
		ret = floatElement->mValue;
	}
	else if (tElement->mType == MUGEN_DEF_SCRIPT_GROUP_VECTOR_ELEMENT) {
		MugenDefScriptVectorElement* vectorElement = (MugenDefScriptVectorElement*)tElement->mData;
		ret = atof(vectorElement->mVector.mElement[0]);
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
	assert(isStringLowercase(tGroupName));
	assert(isStringLowercase(tVariableName));
	if (!stl_string_map_contains_array(tScript->mGroups, tGroupName)) {
		return 0;
	}
	MugenDefScriptGroup* e = &tScript->mGroups[tGroupName];

	return isMugenDefNumberVariableAsGroup(e, tVariableName);
}


int getMugenDefNumberVariable(MugenDefScript * tScript, const char * tGroupName, const char * tVariableName)
{
	assert(isStringLowercase(tGroupName));
	assert(isStringLowercase(tVariableName));
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
	return tElement->mType == MUGEN_DEF_SCRIPT_GROUP_NUMBER_ELEMENT || tElement->mType == MUGEN_DEF_SCRIPT_GROUP_FLOAT_ELEMENT || tElement->mType == MUGEN_DEF_SCRIPT_GROUP_VECTOR_ELEMENT;
}

int getMugenDefNumberVariableAsElement(MugenDefScriptGroupElement * tElement)
{
	assert(tElement->mType == MUGEN_DEF_SCRIPT_GROUP_NUMBER_ELEMENT || tElement->mType == MUGEN_DEF_SCRIPT_GROUP_FLOAT_ELEMENT || tElement->mType == MUGEN_DEF_SCRIPT_GROUP_VECTOR_ELEMENT);

	int ret;
	if (tElement->mType == MUGEN_DEF_SCRIPT_GROUP_NUMBER_ELEMENT) {
		MugenDefScriptNumberElement* numberElement = (MugenDefScriptNumberElement*)tElement->mData;
		ret = numberElement->mValue;
	}
	else if (tElement->mType == MUGEN_DEF_SCRIPT_GROUP_FLOAT_ELEMENT) {
		MugenDefScriptFloatElement* floatElement = (MugenDefScriptFloatElement*)tElement->mData;
		ret = int(floatElement->mValue);
	}
	else if (tElement->mType == MUGEN_DEF_SCRIPT_GROUP_VECTOR_ELEMENT) {
		MugenDefScriptVectorElement* vectorElement = (MugenDefScriptVectorElement*)tElement->mData;
		ret = atoi(vectorElement->mVector.mElement[0]);
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
	assert(isStringLowercase(tGroupName));
	assert(isStringLowercase(tVariableName));
	if (!stl_string_map_contains_array(tScript->mGroups, tGroupName)) {
		return 0;
	}
	MugenDefScriptGroup* e = &tScript->mGroups[tGroupName];

	return isMugenDefVectorVariableAsGroup(e, tVariableName);
}

Vector3D getMugenDefVectorVariable(MugenDefScript * tScript, const char * tGroupName, const char * tVariableName)
{
	assert(isStringLowercase(tGroupName));
	assert(isStringLowercase(tVariableName));
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
		ret = Vector3D(x, y, z);
	} else if (tElement->mType == MUGEN_DEF_SCRIPT_GROUP_FLOAT_ELEMENT) {
		MugenDefScriptFloatElement* floatElement = (MugenDefScriptFloatElement*)tElement->mData;
		double x = floatElement->mValue;
		double y = 0;
		double z = 0;
		ret = Vector3D(x, y, z);
	}
	else if (tElement->mType == MUGEN_DEF_SCRIPT_GROUP_NUMBER_ELEMENT) {
		MugenDefScriptNumberElement* numberElement = (MugenDefScriptNumberElement*)tElement->mData;
		double x = numberElement->mValue;
		double y = 0;
		double z = 0;
		ret = Vector3D(x, y, z);
	}
	else {
		ret = Vector3D(0, 0, 0);
		logError("Unknown type.");
		logErrorInteger(tElement->mType);
		recoverFromError();
	}

	return ret;
}

Vector2D getMugenDefVector2DVariable(MugenDefScript * tScript, const char * tGroupName, const char * tVariableName)
{
	assert(isStringLowercase(tGroupName));
	assert(isStringLowercase(tVariableName));
	assert(stl_string_map_contains_array(tScript->mGroups, tGroupName));
	MugenDefScriptGroup* e = &tScript->mGroups[tGroupName];
	return getMugenDefVector2DVariableAsGroup(e, tVariableName);
}

Vector2D getMugenDefVector2DVariableAsGroup(MugenDefScriptGroup* tGroup, const char* tVariableName) {
	assert(stl_string_map_contains_array(tGroup->mElements, tVariableName));
	MugenDefScriptGroupElement* element = &tGroup->mElements[tVariableName];

	return getMugenDefVector2DVariableAsElement(element);
}

Vector2D getMugenDefVector2DVariableAsElement(MugenDefScriptGroupElement * tElement)
{
	Vector2D ret;
	if (tElement->mType == MUGEN_DEF_SCRIPT_GROUP_VECTOR_ELEMENT) {
		MugenDefScriptVectorElement* vectorElement = (MugenDefScriptVectorElement*)tElement->mData;
		double x = atof(vectorElement->mVector.mElement[0]);
		double y = vectorElement->mVector.mSize >= 2 ? atof(vectorElement->mVector.mElement[1]) : 0;
		ret = Vector2D(x, y);
	}
	else if (tElement->mType == MUGEN_DEF_SCRIPT_GROUP_FLOAT_ELEMENT) {
		MugenDefScriptFloatElement* floatElement = (MugenDefScriptFloatElement*)tElement->mData;
		double x = floatElement->mValue;
		ret = Vector2D(x, 0);
	}
	else if (tElement->mType == MUGEN_DEF_SCRIPT_GROUP_NUMBER_ELEMENT) {
		MugenDefScriptNumberElement* numberElement = (MugenDefScriptNumberElement*)tElement->mData;
		double x = numberElement->mValue;
		ret = Vector2D(x, 0);
	}
	else {
		ret = Vector2D(0, 0);
		logError("Unknown type.");
		logErrorInteger(tElement->mType);
		recoverFromError();
	}

	return ret;
}

int isMugenDefVectorIVariable(MugenDefScript * tScript, const char * tGroupName, const char * tVariableName)
{
	assert(isStringLowercase(tGroupName));
	assert(isStringLowercase(tVariableName));
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
		ret = Vector3DI(x, y, z);
	}
	else if (element->mType == MUGEN_DEF_SCRIPT_GROUP_NUMBER_ELEMENT) {
		MugenDefScriptNumberElement* numberElement = (MugenDefScriptNumberElement*)element->mData;
		int x = numberElement->mValue;
		int y = 0;
		int z = 0;
		ret = Vector3DI(x, y, z);
	}
	else {
		ret = Vector3DI(0, 0, 0);
		logError("Unknown type.");
		logErrorInteger(element->mType);
		recoverFromError();
	}

	return ret;
}

Vector3DI getMugenDefVectorIVariable(MugenDefScript * tScript, const char * tGroupName, const char * tVariableName)
{
	assert(isStringLowercase(tGroupName));
	assert(isStringLowercase(tVariableName));
	assert(stl_string_map_contains_array(tScript->mGroups, tGroupName));
	MugenDefScriptGroup* e = &tScript->mGroups[tGroupName];

	return getMugenDefVectorIVariableAsGroup(e, tVariableName);
}

int isMugenDefVector2DIVariable(MugenDefScript* tScript, const char* tGroupName, const char* tVariableName)
{
	return isMugenDefVectorIVariable(tScript, tGroupName, tVariableName);
}

int isMugenDefVector2DIVariableAsGroup(MugenDefScriptGroup* tGroup, const char* tVariableName)
{
	return isMugenDefVectorIVariableAsGroup(tGroup, tVariableName);
}

Vector2DI getMugenDefVector2DIVariableAsGroup(MugenDefScriptGroup* tGroup, const char* tVariableName)
{
	assert(stl_string_map_contains_array(tGroup->mElements, tVariableName));
	MugenDefScriptGroupElement* element = &tGroup->mElements[tVariableName];

	Vector2DI ret;
	if (element->mType == MUGEN_DEF_SCRIPT_GROUP_VECTOR_ELEMENT) {
		MugenDefScriptVectorElement* vectorElement = (MugenDefScriptVectorElement*)element->mData;
		int x = atoi(vectorElement->mVector.mElement[0]);
		int y = vectorElement->mVector.mSize >= 2 ? atoi(vectorElement->mVector.mElement[1]) : 0;
		ret = Vector2DI(x, y);
	}
	else if (element->mType == MUGEN_DEF_SCRIPT_GROUP_NUMBER_ELEMENT) {
		MugenDefScriptNumberElement* numberElement = (MugenDefScriptNumberElement*)element->mData;
		int x = numberElement->mValue;
		int y = 0;
		ret = Vector2DI(x, y);
	}
	else {
		ret = Vector2DI(0, 0);
		logError("Unknown type.");
		logErrorInteger(element->mType);
		recoverFromError();
	}

	return ret;
}

Vector2DI getMugenDefVector2DIVariable(MugenDefScript* tScript, const char* tGroupName, const char* tVariableName)
{
	assert(isStringLowercase(tGroupName));
	assert(isStringLowercase(tVariableName));
	assert(stl_string_map_contains_array(tScript->mGroups, tGroupName));
	MugenDefScriptGroup* e = &tScript->mGroups[tGroupName];
	return getMugenDefVector2DIVariableAsGroup(e, tVariableName);
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
	assert(isStringLowercase(tGroupName));
	assert(isStringLowercase(tVariableName));
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

int isMugenDefGeoRectangle2DVariable(MugenDefScript* tScript, const char* tGroupName, const char* tVariableName) {
	assert(isStringLowercase(tGroupName));
	assert(isStringLowercase(tVariableName));
	if (!stl_string_map_contains_array(tScript->mGroups, tGroupName)) {
		return 0;
	}
	MugenDefScriptGroup* e = &tScript->mGroups[tGroupName];
	return isMugenDefGeoRectangle2DVariableAsGroup(e, tVariableName);
}

int isMugenDefGeoRectangle2DVariableAsGroup(MugenDefScriptGroup* tGroup, const char* tVariableName) {
	if (!stl_string_map_contains_array(tGroup->mElements, tVariableName)) {
		return 0;
	}
	MugenDefScriptGroupElement* element = &tGroup->mElements[tVariableName];
	return isMugenDefGeoRectangle2DVariableAsElement(element);
}

int isMugenDefGeoRectangle2DVariableAsElement(MugenDefScriptGroupElement * tElement) {
	if (tElement->mType != MUGEN_DEF_SCRIPT_GROUP_VECTOR_ELEMENT) return 0;

	MugenDefScriptVectorElement* vectorElement = (MugenDefScriptVectorElement*)tElement->mData;
	return vectorElement->mVector.mSize >= 4;
}

GeoRectangle2D getMugenDefGeoRectangle2DVariable(MugenDefScript* tScript, const char* tGroupName, const char* tVariableName) {
	assert(isStringLowercase(tGroupName));
	assert(isStringLowercase(tVariableName));
	assert(stl_string_map_contains_array(tScript->mGroups, tGroupName));
	MugenDefScriptGroup* e = &tScript->mGroups[tGroupName];
	return getMugenDefGeoRectangle2DVariableAsGroup(e, tVariableName);
}
GeoRectangle2D getMugenDefGeoRectangle2DVariableAsGroup(MugenDefScriptGroup* tGroup, const char* tVariableName) {
	assert(stl_string_map_contains_array(tGroup->mElements, tVariableName));
	MugenDefScriptGroupElement* element = &tGroup->mElements[tVariableName];
	return getMugenDefGeoRectangle2DVariableAsElement(element);
}
GeoRectangle2D getMugenDefGeoRectangle2DVariableAsElement(MugenDefScriptGroupElement * tElement) {
	assert(tElement->mType == MUGEN_DEF_SCRIPT_GROUP_VECTOR_ELEMENT);
	MugenDefScriptVectorElement* vectorElement = (MugenDefScriptVectorElement*)tElement->mData;
	assert(vectorElement->mVector.mSize >= 4);
	GeoRectangle2D ret;
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
		ret.mVector.mElement[i] = (char*)allocMemory(int(strlen(vectorElement->mVector.mElement[i]) + 10));
		strcpy(ret.mVector.mElement[i], vectorElement->mVector.mElement[i]);
	}

	return ret.mVector;
}

void getMugenDefStringOrDefault(char* tDst, MugenDefScript* s, const char* tGroup, const char* tVariable, const char* tDefault) {
	assert(isStringLowercase(tGroup));
	assert(isStringLowercase(tVariable));
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
	char* ret = (char*)allocMemory(int(strlen(tString) + 1));
	strcpy(ret, tString);
	return ret;
}

char* getAllocatedMugenDefStringOrDefault(MugenDefScript* tScript, const char* tGroupName, const char* tVariableName, const char* tDefault) {
	assert(isStringLowercase(tGroupName));
	assert(isStringLowercase(tVariableName));
	if (isMugenDefStringVariable(tScript, tGroupName, tVariableName)) return getAllocatedMugenDefStringVariable(tScript, tGroupName, tVariableName);
	else return createAllocatedString(tDefault);
}

char* getAllocatedMugenDefStringOrDefaultAsGroup(MugenDefScriptGroup* tGroup, const char* tVariable, const char* tDefault) {
	if (isMugenDefStringVariableAsGroup(tGroup, tVariable)) return getAllocatedMugenDefStringVariableAsGroup(tGroup, tVariable);
	else return createAllocatedString(tDefault);
}

std::string getSTLMugenDefStringOrDefault(MugenDefScript * s, const char * tGroup, const char * tVariable, const char * tDefault)
{
	assert(isStringLowercase(tGroup));
	assert(isStringLowercase(tVariable));
	if (isMugenDefStringVariable(s, tGroup, tVariable)) {
		return getSTLMugenDefStringVariable(s, tGroup, tVariable);
	}
	else {
		return tDefault;
	}
}

std::string getSTLMugenDefStringOrDefaultAsGroup(MugenDefScriptGroup * tGroup, const char * tVariable, const char * tDefault)
{
	if (isMugenDefStringVariableAsGroup(tGroup, tVariable)) {
		return getSTLMugenDefStringVariableAsGroup(tGroup, tVariable);
	}
	else {
		return tDefault;
	}
}

double getMugenDefFloatOrDefault(MugenDefScript* s, const char* tGroup, const char* tVariable, double tDefault) {
	assert(isStringLowercase(tGroup));
	assert(isStringLowercase(tVariable));
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
	assert(isStringLowercase(tGroup));
	assert(isStringLowercase(tVariable));
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

Vector3D getMugenDefVectorOrDefault(MugenDefScript* s, const char* tGroup, const char* tVariable, const Vector3D& tDefault) {
	assert(isStringLowercase(tGroup));
	assert(isStringLowercase(tVariable));
	if (isMugenDefVectorVariable(s, tGroup, tVariable)) {
		return getMugenDefVectorVariable(s, tGroup, tVariable);
	}
	else {
		return tDefault;
	}
}

Vector3D getMugenDefVectorOrDefaultAsGroup(MugenDefScriptGroup* tGroup, const char* tVariable, const Vector3D& tDefault) {
	if (isMugenDefVectorVariableAsGroup(tGroup, tVariable)) {
		return getMugenDefVectorVariableAsGroup(tGroup, tVariable);
	}
	else {
		return tDefault;
	}
}

Vector2D getMugenDefVector2DOrDefault(MugenDefScript * s, const char * tGroup, const char * tVariable, const Vector2D & tDefault)
{
	assert(isStringLowercase(tGroup));
	assert(isStringLowercase(tVariable));
	if (isMugenDefVectorVariable(s, tGroup, tVariable)) {
		return getMugenDefVector2DVariable(s, tGroup, tVariable);
	}
	else {
		return tDefault;
	}
}

Vector2D getMugenDefVector2DOrDefaultAsGroup(MugenDefScriptGroup * tGroup, const char * tVariable, const Vector2D & tDefault)
{
	if (isMugenDefVectorVariableAsGroup(tGroup, tVariable)) {
		return getMugenDefVector2DVariableAsGroup(tGroup, tVariable);
	}
	else {
		return tDefault;
	}
}

Vector3DI getMugenDefVectorIOrDefault(MugenDefScript* s, const char* tGroup, const char* tVariable, const Vector3DI& tDefault) {
	assert(isStringLowercase(tGroup));
	assert(isStringLowercase(tVariable));
	if (isMugenDefVectorIVariable(s, tGroup, tVariable)) {
		return getMugenDefVectorIVariable(s, tGroup, tVariable);
	}
	else {
		return tDefault;
	}
}

Vector3DI getMugenDefVectorIOrDefaultAsGroup(MugenDefScriptGroup* tGroup, const char* tVariable, const Vector3DI& tDefault) {
	if (isMugenDefVectorIVariableAsGroup(tGroup, tVariable)) {
		return getMugenDefVectorIVariableAsGroup(tGroup, tVariable);
	}
	else {
		return tDefault;
	}
}

Vector2DI getMugenDefVector2DIOrDefault(MugenDefScript* s, const char* tGroup, const char* tVariable, const Vector2DI& tDefault)
{
	assert(isStringLowercase(tGroup));
	assert(isStringLowercase(tVariable));
	if (isMugenDefVector2DIVariable(s, tGroup, tVariable)) {
		return getMugenDefVector2DIVariable(s, tGroup, tVariable);
	}
	else {
		return tDefault;
	}
}

Vector2DI getMugenDefVector2DIOrDefaultAsGroup(MugenDefScriptGroup* tGroup, const char* tVariable, const Vector2DI& tDefault)
{
	if (isMugenDefVector2DIVariableAsGroup(tGroup, tVariable)) {
		return getMugenDefVector2DIVariableAsGroup(tGroup, tVariable);
	}
	else {
		return tDefault;
	}
}

GeoRectangle2D getMugenDefGeoRectangle2DOrDefault(MugenDefScript* s, const char* tGroup, const char* tVariable, const GeoRectangle2D& tDefault) {
	assert(isStringLowercase(tGroup));
	assert(isStringLowercase(tVariable));
	if (isMugenDefGeoRectangle2DVariable(s, tGroup, tVariable)) {
		return getMugenDefGeoRectangle2DVariable(s, tGroup, tVariable);
	}
	else {
		return tDefault;
	}
}

GeoRectangle2D getMugenDefGeoRectangle2DOrDefaultAsGroup(MugenDefScriptGroup* tGroup, const char* tVariable, const GeoRectangle2D& tDefault) {
	if (isMugenDefGeoRectangle2DVariableAsGroup(tGroup, tVariable)) {
		return getMugenDefGeoRectangle2DVariableAsGroup(tGroup, tVariable);
	}
	else {
		return tDefault;
	}
}
