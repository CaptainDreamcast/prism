#pragma once

#include "datastructures.h"
#include "geometry.h"

#define MUGEN_DEF_STRING_LENGTH 500

typedef enum {
	MUGEN_DEF_SCRIPT_GROUP_STRING_ELEMENT,
	MUGEN_DEF_SCRIPT_GROUP_NUMBER_ELEMENT,
	MUGEN_DEF_SCRIPT_GROUP_FLOAT_ELEMENT,
	MUGEN_DEF_SCRIPT_GROUP_VECTOR_ELEMENT,
} MugenDefScriptGroupElementType;

typedef struct {
	int mSize;
	char** mElement;

} MugenStringVector;

typedef struct {
	MugenStringVector mVector;
} MugenDefScriptVectorElement;

typedef struct {
	char* mString;

} MugenDefScriptStringElement;

typedef struct {
	int mValue;
} MugenDefScriptNumberElement;

typedef struct {
	double mValue;
} MugenDefScriptFloatElement;

typedef struct {
	char mName[100];
	MugenDefScriptGroupElementType mType;
	void* mData;

} MugenDefScriptGroupElement;

typedef struct MugenDefScriptGroup_t{
	char mName[100];
	StringMap mElements;
	List mOrderedElementList;
	struct MugenDefScriptGroup_t* mNext;
} MugenDefScriptGroup;

typedef struct {
	MugenDefScriptGroup* mFirstGroup;
	StringMap mGroups;
} MugenDefScript;

fup MugenDefScript loadMugenDefScript(char* tPath);
fup void unloadMugenDefScript(MugenDefScript tScript);

fup int isMugenDefStringVariable(MugenDefScript* tScript, char* tGroupName, char* tVariableName);
fup char* getAllocatedMugenDefStringVariable(MugenDefScript* tScript, char* tGroupName, char* tVariableName);
fup char* getAllocatedMugenDefStringVariableAsGroup(MugenDefScriptGroup* tGroup, char* tVariableName);
fup int isMugenDefStringVariableAsElement(MugenDefScriptGroupElement* tElement);
fup char* getAllocatedMugenDefStringVariableAsElement(MugenDefScriptGroupElement* tElement);

fup int isMugenDefVariable(MugenDefScript* tScript, char* tGroupName, char* tVariableName);

fup int isMugenDefFloatVariable(MugenDefScript* tScript, char* tGroupName, char* tVariableName);
fup double getMugenDefFloatVariable(MugenDefScript* tScript, char* tGroupName, char* tVariableName);
fup double getMugenDefFloatVariableAsGroup(MugenDefScriptGroup* tGroup, char* tVariableName);
fup int isMugenDefFloatVariableAsElement(MugenDefScriptGroupElement* tElement);
fup double getMugenDefFloatVariableAsElement(MugenDefScriptGroupElement* tElement);


fup int isMugenDefNumberVariable(MugenDefScript* tScript, char* tGroupName, char* tVariableName);
fup int getMugenDefNumberVariable(MugenDefScript* tScript, char* tGroupName, char* tVariableName);
fup int getMugenDefNumberVariableAsGroup(MugenDefScriptGroup* tGroup, char* tVariableName);
fup int isMugenDefNumberVariableAsElement(MugenDefScriptGroupElement* tElement);
fup int getMugenDefNumberVariableAsElement(MugenDefScriptGroupElement* tElement);

fup int isMugenDefVectorVariable(MugenDefScript* tScript, char* tGroupName, char* tVariableName);
fup Vector3D getMugenDefVectorVariable(MugenDefScript* tScript, char* tGroupName, char* tVariableName);
fup int isMugenDefVectorVariableAsGroup(MugenDefScriptGroup* tGroup, char* tVariableName);
fup Vector3D getMugenDefVectorVariableAsGroup(MugenDefScriptGroup* tGroup, char* tVariableName);
fup int isMugenDefVectorVariableAsElement(MugenDefScriptGroupElement* tElement);
fup Vector3D getMugenDefVectorVariableAsElement(MugenDefScriptGroupElement* tElement);

fup int isMugenDefVectorIVariable(MugenDefScript* tScript, char* tGroupName, char* tVariableName);
fup Vector3DI getMugenDefVectorIVariableAsGroup(MugenDefScriptGroup* tGroup, char* tVariableName);
fup Vector3DI getMugenDefVectorIVariable(MugenDefScript* tScript, char* tGroupName, char* tVariableName);

fup int isMugenDefStringVectorVariableAsElement(MugenDefScriptGroupElement* tElement);
fup MugenStringVector getMugenDefStringVectorVariableAsElement(MugenDefScriptGroupElement* tElement);
fup MugenStringVector copyMugenDefStringVectorVariableAsElement(MugenDefScriptGroupElement * tElement);

fup void getMugenDefStringOrDefault(char* tDst, MugenDefScript* s, char* tGroup, char* tVariable, char* tDefault);

fup void getMugenDefFloatOrDefault(double* tDst, MugenDefScript* s, char* tGroup, char* tVariable, double tDefault);
fup void getMugenDefIntegerOrDefault(int* tDst, MugenDefScript* s, char* tGroup, char* tVariable, int tDefault);
fup void getMugenDefVectorOrDefault(Vector3D* tDst, MugenDefScript* s, char* tGroup, char* tVariable, Vector3D tDefault);
fup void getMugenDefVectorOrDefaultAsGroup(Vector3D* tDst, MugenDefScriptGroup* tGroup, char* tVariable, Vector3D tDefault);
fup void getMugenDefVectorIOrDefault(Vector3DI* tDst, MugenDefScript* s, char* tGroup, char* tVariable, Vector3DI tDefault);