#ifndef TARI_SCRIPT_H
#define TARI_SCRIPT_H

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

fup Script loadScript(char* tPath);

fup void executeOnScriptRegion(ScriptRegion tRegion, ScriptExecuteCB tFunc, void* tCaller);

fup ScriptPosition getNextScriptString(ScriptPosition tPos, char* tDest);
fup ScriptPosition getNextScriptDouble(ScriptPosition tPos, double* tDest);
fup ScriptPosition getNextScriptInteger(ScriptPosition tPos, int* tDest);
fup ScriptRegion getScriptRegion(Script tScript, char* tName);
fup ScriptPosition getScriptRegionStart(ScriptRegion tRegion);
fup ScriptPosition getPositionAfterScriptRegion(ScriptRegion tRegion, ScriptRegion tSkippedRegion);
fup ScriptRegion getScriptRegionAtPosition(ScriptPosition tPos);
fup int hasNextScriptWord(ScriptPosition tPos);
fup ScriptPosition getNextScriptInstruction(ScriptPosition tPos);

#endif
