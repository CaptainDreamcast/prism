#include "include/script.h"

#include <string.h>

#include "include/log.h"
#include "include/system.h"


Script loadScript(char* tPath) {
	Script ret;

	ret.mBuffer = fileToBuffer(tPath);
	appendTerminationSymbolToBuffer(&ret.mBuffer);
	return ret;
}

void executeOnScriptRegion(ScriptRegion tRegion, ScriptExecuteCB tFunc, void* tCaller) {
	ScriptPosition pos = getScriptRegionStart(tRegion);
	while(pos.mPointer != NULL) {
		pos = tFunc(tCaller, pos);
		if(pos.mPointer == NULL) break;
		pos = getNextScriptInstruction(pos);
	}
}

static ScriptPosition updateScriptPositionValidity(ScriptPosition tPos) {

	
	if((tPos.mPointer >= tPos.mRegion.mEnd)) {
		tPos.mPointer = NULL;
	}

	return tPos;
}

ScriptPosition getNextScriptString(ScriptPosition tPos, char* tDest) {
	int positionsRead;	
	sscanf(tPos.mPointer, "%s%n", tDest, &positionsRead);
	tPos.mPointer += positionsRead;
	tPos = updateScriptPositionValidity(tPos);
	
	return tPos;
}

static ScriptPosition skipNextScriptString(ScriptPosition tPos) {
	char buf[100];
	tPos = getNextScriptString(tPos, buf);
	return tPos;
}

static ScriptPosition getNextScriptRawCharacter(ScriptPosition tPos, char* tDest) {	
	tDest[0] = *tPos.mPointer;
	tPos.mPointer++;
	tPos = updateScriptPositionValidity(tPos);
	
	return tPos;
}

ScriptPosition getNextScriptDouble(ScriptPosition tPos, double* tDest) {
	int positionsRead;	
	sscanf(tPos.mPointer, "%lf%n", tDest, &positionsRead);
	tPos.mPointer += positionsRead;
	tPos = updateScriptPositionValidity(tPos);
	
	return tPos;
}

ScriptPosition getNextScriptInteger(ScriptPosition tPos, int* tDest) {
	int positionsRead;	
	sscanf(tPos.mPointer, "%d%n", tDest, &positionsRead);
	tPos.mPointer += positionsRead;
	tPos = updateScriptPositionValidity(tPos);
	
	return tPos;
}

static ScriptRegion makeScriptRegion(Script tScript, char* tStart, char* tEnd) {
	ScriptRegion ret;
	ret.mScript = tScript;
	ret.mStart = tStart;
	ret.mEnd = tEnd;
	return ret;
}

static ScriptRegion getWholeScriptRegion(Script tScript) {
	return makeScriptRegion(tScript, tScript.mBuffer.mData, tScript.mBuffer.mData + tScript.mBuffer.mLength - 1);
}

static ScriptPosition findNextScriptOccurenceOnSameLevel(ScriptPosition tPos, char* tWord) {
	char w[100];

	int isInside = 0;

	while(tPos.mPointer != NULL) {
		ScriptPosition next;
		next = getNextScriptString(tPos, w);
		if(!isInside && !strcmp(tWord, w)) {
			return tPos;
		}

		if(!strcmp("{",  w)) isInside++;
		if(!strcmp("}",  w)) isInside--;

		tPos = next;
	}

	if(*tWord == '}') {
		// TODO: unhack
		tPos.mPointer = tPos.mRegion.mEnd;
		return tPos;
	}

	

	logError("Unable to find word");
	logString(tPos.mPointer);
	logString(tWord);
	abortSystem();
	return tPos;
}

static ScriptPosition findNextCharacterScriptOccurenceOnSameLevel(ScriptPosition tPos, char tChar) {
	char w[100];

	int isInside = 0;

	while(tPos.mPointer != NULL) {
		ScriptPosition next;
		next = getNextScriptRawCharacter(tPos, w);
		if(!isInside && tChar == *w) {
			return tPos;
		}

		if(!strcmp("{",  w)) isInside++;
		if(!strcmp("}",  w)) isInside--;

		tPos = next;
	}

	logError("Unable to find char");
	logString(tPos.mPointer);
	logInteger(tChar);
	abortSystem();
	return tPos;
}

static ScriptPosition findScriptRegionEnd(ScriptPosition tPos) {
	tPos = findNextScriptOccurenceOnSameLevel(tPos, "}");
	return tPos;
}

static ScriptPosition findScriptRegionStart(ScriptPosition tPos, char* tName) {
	char w1[100];
	char w2[100];

	int isInside = 0;

	while(tPos.mPointer != NULL) {
		ScriptPosition next;
		tPos = getNextScriptString(tPos, w1);
		next = getNextScriptString(tPos, w2);
		if(!isInside && !strcmp(tName, w1) && w2[0] == '{') {
			if(next.mPointer == NULL) {
				next.mPointer = tPos.mRegion.mScript.mBuffer.mData + tPos.mRegion.mScript.mBuffer.mLength - 1;
			}
			return next;
		}

		if(!strcmp("{",  w1)) isInside++;
		if(!strcmp("}",  w1)) isInside--;
	}

	logError("Unable to find name");
	logString(tPos.mPointer);
	logString(tName);
	abortSystem();
	return tPos;
}

ScriptRegion getScriptRegion(Script tScript, char* tName) {
	ScriptRegion r = getWholeScriptRegion(tScript);
	ScriptPosition p0 = getScriptRegionStart(r);
	ScriptPosition p1 = findScriptRegionStart(p0, tName);
	ScriptPosition p2 = findScriptRegionEnd(p1);
	p2.mPointer--;

	return makeScriptRegion(tScript, p1.mPointer, p2.mPointer);
}

static ScriptPosition makeScriptPosition(ScriptRegion tRegion, char* tPointer) {
	ScriptPosition ret;
	ret.mRegion = tRegion;
	ret.mPointer = tPointer;
	return ret;
}

ScriptPosition getScriptRegionStart(ScriptRegion tRegion) {
	return makeScriptPosition(tRegion, tRegion.mStart);

}
ScriptPosition getPositionAfterScriptRegion(ScriptRegion tRegion, ScriptRegion tSkippedRegion) {
	ScriptPosition ret = makeScriptPosition(tRegion, tSkippedRegion.mEnd);
	ret = skipNextScriptString(ret);
	return ret;
}

ScriptRegion getScriptRegionAtPosition(ScriptPosition tPos) {
	ScriptPosition start = findNextScriptOccurenceOnSameLevel(tPos, "{");
	start = skipNextScriptString(start);
	ScriptPosition end = findNextScriptOccurenceOnSameLevel(start, "}");

	return makeScriptRegion(tPos.mRegion.mScript, start.mPointer, end.mPointer);
}

int hasNextScriptWord(ScriptPosition tPos) {
	return tPos.mPointer != NULL;
}

ScriptPosition getNextScriptInstruction(ScriptPosition tPos) {
	if(tPos.mPointer == NULL) return tPos;

	tPos = findNextCharacterScriptOccurenceOnSameLevel(tPos, '\n');
	tPos.mPointer++;
	tPos = updateScriptPositionValidity(tPos);
	return tPos;
}

