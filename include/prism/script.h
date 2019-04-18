#pragma once

#include "file.h"

typedef struct {
	Buffer mBuffer;
} Script;

typedef struct {
	Script mScript;
	char* mStart;
	char* mEnd;
} ScriptRegion;

typedef struct {
	ScriptRegion mRegion;
	char* mPointer;

} ScriptPosition;

typedef ScriptPosition (*ScriptExecuteCB)(void* tCaller, ScriptPosition tPosition);

Script loadScript(char* tPath);

void executeOnScriptRegion(ScriptRegion tRegion, ScriptExecuteCB tFunc, void* tCaller);

ScriptPosition getNextScriptString(ScriptPosition tPos, char* tDest);
ScriptPosition getNextScriptDouble(ScriptPosition tPos, double* tDest);
ScriptPosition getNextScriptInteger(ScriptPosition tPos, int* tDest);
ScriptRegion getScriptRegion(Script tScript, const char* tName);
ScriptPosition getScriptRegionStart(ScriptRegion tRegion);
ScriptPosition getPositionAfterScriptRegion(ScriptRegion tRegion, ScriptRegion tSkippedRegion);
ScriptRegion getScriptRegionAtPosition(ScriptPosition tPos);
int hasNextScriptWord(ScriptPosition tPos);
ScriptPosition getNextScriptInstruction(ScriptPosition tPos);

