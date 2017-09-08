#pragma once

#include "mugendefreader.h"

fup void resetMugenScriptParser();
fup void addMugenScriptParseFunction(int(*tIsFunc)(MugenDefScriptGroup*), void(*tHandleFunc)(MugenDefScriptGroup*));
fup void parseMugenScript(MugenDefScript* tScript);