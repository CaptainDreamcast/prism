#pragma once

#include "file.h"

struct ModifiableMugenDefScript
{
	Buffer mOwnedBuffer;
};

ModifiableMugenDefScript openModifiableMugenDefScript(const std::string& tPath);
void closeModifiableMugenDefScript(ModifiableMugenDefScript* tScript);
void saveModifiableMugenDefScript(ModifiableMugenDefScript* tScript, const std::string& tPath);
void clearModifiableMugenDefScript(ModifiableMugenDefScript* tScript);

void saveMugenDefString(ModifiableMugenDefScript* tScript, const char* tGroupName, const char* tVariableName, const std::string& tValue);
void saveMugenDefFloat(ModifiableMugenDefScript* tScript, const char* tGroupName, const char* tVariableName, double tValue);
void saveMugenDefInteger(ModifiableMugenDefScript* tScript, const char* tGroupName, const char* tVariableName, int tValue);