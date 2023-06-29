#include "prism/script.h"

#include <string.h>

#include "prism/log.h"
#include "prism/system.h"


Script loadScript(const char* tPath) {
	Script ret;

	ret.mBuffer = fileToBuffer(tPath);
	appendTerminationSymbolToBuffer(&ret.mBuffer);
	return ret;
}

void executeOnScriptRegion(const ScriptRegion& tRegion, ScriptExecuteCB tFunc, void* tCaller) {
	ScriptPosition pos = getScriptRegionStart(tRegion);
	while(pos.mPointer != NULL) {
		pos = tFunc(tCaller, pos);
		if(pos.mPointer == NULL) break;
		pos = getNextScriptInstruction(pos);
	}
}

static ScriptPosition updateScriptPositionValidity(const ScriptPosition& tPos) {

	auto ret = tPos;
	if((ret.mPointer >= ret.mRegion.mEnd)) {
		ret.mPointer = NULL;
	}

	return ret;
}

ScriptPosition getNextScriptString(const ScriptPosition& tPos, char* tDest) {
	int positionsRead;	
	int items = sscanf(tPos.mPointer, "%99s%n", tDest, &positionsRead);
	if (items != 1) {
		logWarningFormat("Unable to parse next script string from: %s", (char*)tPos.mPointer);
	}
	auto ret = tPos;
	ret.mPointer += positionsRead;
	ret = updateScriptPositionValidity(ret);
	
	return ret;
}

static ScriptPosition skipNextScriptString(const ScriptPosition& tPos) {
	char buf[100];
	return getNextScriptString(tPos, buf);
}

static ScriptPosition getNextScriptRawCharacter(const ScriptPosition& tPos, char* tDest) {
	tDest[0] = *tPos.mPointer;
	auto ret = tPos;
	ret.mPointer++;
	ret = updateScriptPositionValidity(ret);

	return ret;
}

ScriptPosition getNextScriptDouble(const ScriptPosition& tPos, double* tDest) {
	int positionsRead;	
	int items = sscanf(tPos.mPointer, "%lf%n", tDest, &positionsRead);
	if (items != 1) {
		logWarningFormat("Unable to parse next script double from: %s", (char*)tPos.mPointer);
	}
	auto ret = tPos;
	ret.mPointer += positionsRead;
	ret = updateScriptPositionValidity(ret);
	
	return ret;
}

ScriptPosition getNextScriptInteger(const ScriptPosition& tPos, int* tDest) {
	int positionsRead;	
	int items = sscanf(tPos.mPointer, "%d%n", tDest, &positionsRead);
	if (items != 1) {
		logWarningFormat("Unable to parse next script integer from: %s", (char*)tPos.mPointer);
	}
	auto ret = tPos;
	ret.mPointer += positionsRead;
	ret = updateScriptPositionValidity(ret);
	
	return ret;
}

static ScriptRegion makeScriptRegion(const Script& tScript, char* tStart, char* tEnd) {
	ScriptRegion ret;
	ret.mScript = tScript;
	ret.mStart = tStart;
	ret.mEnd = tEnd;
	return ret;
}

static ScriptRegion getWholeScriptRegion(Script tScript) {
	return makeScriptRegion(tScript, (char*)tScript.mBuffer.mData, (char*)((uintptr_t)tScript.mBuffer.mData + tScript.mBuffer.mLength - 1));
}

static ScriptPosition findNextScriptOccurenceOnSameLevel(const ScriptPosition& tPos, const char* tWord) {
	char w[100];

	int isInside = 0;

	auto ret = tPos;
	while(ret.mPointer != NULL) {
		ScriptPosition next;
		next = getNextScriptString(ret, w);
		if(!isInside && !strcmp(tWord, w)) {
			return ret;
		}

		if(!strcmp("{",  w)) isInside++;
		if(!strcmp("}",  w)) isInside--;

		ret = next;
	}

	if(*tWord == '}') {
		ret.mPointer = ret.mRegion.mEnd;
		return ret;
	}

	logError("Unable to find word");
	logString(ret.mPointer);
	logString(tWord);
	recoverFromError();
	return ret;
}

static ScriptPosition findNextCharacterScriptOccurenceOnSameLevel(const ScriptPosition& tPos, char tChar) {
	char w[100];

	int isInside = 0;

	auto ret = tPos;
	while(ret.mPointer != NULL) {
		ScriptPosition next;
		next = getNextScriptRawCharacter(ret, w);
		if(!isInside && tChar == *w) {
			return ret;
		}

		if(!strcmp("{",  w)) isInside++;
		if(!strcmp("}",  w)) isInside--;

		ret = next;
	}

	logError("Unable to find char");
	logString(ret.mPointer);
	logInteger(tChar);
	recoverFromError();
	return ret;

}

static ScriptPosition findScriptRegionEnd(const ScriptPosition& tPos) {
	return findNextScriptOccurenceOnSameLevel(tPos, "}");
}

static ScriptPosition findScriptRegionStart(const ScriptPosition& tPos, const char* tName) {
	char w1[100];
	char w2[100];

	int isInside = 0;

	auto ret = tPos;
	while(ret.mPointer != NULL) {
		ScriptPosition next;
		ret = getNextScriptString(ret, w1);
		next = getNextScriptString(ret, w2);
		if(!isInside && !strcmp(tName, w1) && w2[0] == '{') {
			if(next.mPointer == NULL) {
				next.mPointer = (char*)((uintptr_t)ret.mRegion.mScript.mBuffer.mData + ret.mRegion.mScript.mBuffer.mLength - 1);
			}
			return next;
		}

		if(!strcmp("{",  w1)) isInside++;
		if(!strcmp("}",  w1)) isInside--;
	}

	logError("Unable to find name");
	logString(ret.mPointer);
	logString(tName);
	recoverFromError();
	#if defined(DREAMCAST) || defined(__EMSCRIPTEN__)
	return ret;
	#endif
}

ScriptRegion getScriptRegion(const Script& tScript, const char* tName) {
	ScriptRegion r = getWholeScriptRegion(tScript);
	ScriptPosition p0 = getScriptRegionStart(r);
	ScriptPosition p1 = findScriptRegionStart(p0, tName);
	ScriptPosition p2 = findScriptRegionEnd(p1);
	p2.mPointer--;

	return makeScriptRegion(tScript, p1.mPointer, p2.mPointer);
}

static ScriptPosition makeScriptPosition(const ScriptRegion& tRegion, char* tPointer) {
	ScriptPosition ret;
	ret.mRegion = tRegion;
	ret.mPointer = tPointer;
	return ret;
}

ScriptPosition getScriptRegionStart(const ScriptRegion& tRegion) {
	return makeScriptPosition(tRegion, tRegion.mStart);

}
ScriptPosition getPositionAfterScriptRegion(const ScriptRegion& tRegion, const ScriptRegion& tSkippedRegion) {
	ScriptPosition ret = makeScriptPosition(tRegion, tSkippedRegion.mEnd);
	ret = skipNextScriptString(ret);
	return ret;
}

ScriptRegion getScriptRegionAtPosition(const ScriptPosition& tPos) {
	ScriptPosition start = findNextScriptOccurenceOnSameLevel(tPos, "{");
	start = skipNextScriptString(start);
	ScriptPosition end = findNextScriptOccurenceOnSameLevel(start, "}");

	return makeScriptRegion(tPos.mRegion.mScript, start.mPointer, end.mPointer);
}

int hasNextScriptWord(const ScriptPosition& tPos) {
	return tPos.mPointer != NULL;
}

ScriptPosition getNextScriptInstruction(const ScriptPosition& tPos) {
	if(tPos.mPointer == NULL) return tPos;
	auto ret = findNextCharacterScriptOccurenceOnSameLevel(tPos, '\n');
	ret.mPointer++;
	ret = updateScriptPositionValidity(ret);
	return ret;
}

