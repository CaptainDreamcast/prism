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

typedef ScriptPosition (*ScriptExecuteCB)(void* tCaller, const ScriptPosition& tPosition);

Script loadScript(const char* tPath);

void executeOnScriptRegion(const ScriptRegion& tRegion, ScriptExecuteCB tFunc, void* tCaller);

ScriptPosition getNextScriptString(const ScriptPosition& tPos, char* tDest);
ScriptPosition getNextScriptDouble(const ScriptPosition& tPos, double* tDest);
ScriptPosition getNextScriptInteger(const ScriptPosition& tPos, int* tDest);
ScriptRegion getScriptRegion(const Script& tScript, const char* tName);
ScriptPosition getScriptRegionStart(const ScriptRegion& tRegion);
ScriptPosition getPositionAfterScriptRegion(const ScriptRegion& tRegion, const ScriptRegion& tSkippedRegion);
ScriptRegion getScriptRegionAtPosition(const ScriptPosition& tPos);
int hasNextScriptWord(const ScriptPosition& tPos);
ScriptPosition getNextScriptInstruction(const ScriptPosition& tPos);

