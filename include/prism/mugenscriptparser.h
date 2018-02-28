#pragma once

#include "mugendefreader.h"

void resetMugenScriptParser();
void addMugenScriptParseFunction(int(*tIsFunc)(MugenDefScriptGroup*), void(*tHandleFunc)(MugenDefScriptGroup*));
void parseMugenScript(MugenDefScript* tScript);