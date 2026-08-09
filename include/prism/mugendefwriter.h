#pragma once

#include <utility>
#include <vector>

#include "file.h"

namespace prism {

struct ModifiableMugenDefScript
{
	Buffer mOwnedBuffer;
};

ModifiableMugenDefScript openModifiableMugenDefScript(const std::string& tPath);
void closeModifiableMugenDefScript(ModifiableMugenDefScript* tScript);
void saveModifiableMugenDefScript(ModifiableMugenDefScript* tScript, const std::string& tPath);
void clearModifiableMugenDefScript(ModifiableMugenDefScript* tScript);

void saveMugenDefString(ModifiableMugenDefScript* tScript, const char* tGroupName, const char* tVariableName, const std::string& tValue);
void saveMugenDefFloat(ModifiableMugenDefScript* tScript, const char* tGroupName, const char* tVariableName, float tValue);
void saveMugenDefInteger(ModifiableMugenDefScript* tScript, const char* tGroupName, const char* tVariableName, int tValue);

void saveMugenDefString(const std::string& tPath, const char* tGroupName, size_t tGroupOffset, const char* tVariableName, const std::string& tValue);

// Inserts a new [tNewGroupName] group with the given key = value lines at the end of the content of an anchor group. The anchor is the tAnchorGroupOffset-th group after the first tAnchorGroupName group (matching the offset semantics of saveMugenDefString), or the tAnchorGroupName group itself when tAnchorGroupOffset < 0. No-op when the anchor cannot be found
void addMugenDefScriptGroup(const std::string& tPath, const char* tAnchorGroupName, int tAnchorGroupOffset, const char* tNewGroupName, const std::vector<std::pair<std::string, std::string>>& tVariables);

}