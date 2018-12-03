#pragma once

#include <string>

#include "datastructures.h"
#include "geometry.h"
#include "file.h"

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

MugenDefScript loadMugenDefScript(std::string& tPath);
MugenDefScript loadMugenDefScript(char* tPath);
MugenDefScript loadMugenDefScriptFromBufferAndFreeBuffer(Buffer tBuffer);
void unloadMugenDefScript(MugenDefScript tScript);

int isMugenDefStringVariable(MugenDefScript* tScript, char* tGroupName, char* tVariableName);
char* getAllocatedMugenDefStringVariable(MugenDefScript* tScript, char* tGroupName, char* tVariableName);
int isMugenDefStringVariableAsGroup(MugenDefScriptGroup* tGroup, char* tVariableName);
char* getAllocatedMugenDefStringVariableAsGroup(MugenDefScriptGroup* tGroup, char* tVariableName);
int isMugenDefStringVariableAsElement(MugenDefScriptGroupElement* tElement);
char* getAllocatedMugenDefStringVariableForAssignmentAsElement(MugenDefScriptGroupElement* tElement);
char* getAllocatedMugenDefStringVariableAsElement(MugenDefScriptGroupElement* tElement);

int isMugenDefVariable(MugenDefScript* tScript, char* tGroupName, char* tVariableName);

int isMugenDefFloatVariable(MugenDefScript* tScript, char* tGroupName, char* tVariableName);
double getMugenDefFloatVariable(MugenDefScript* tScript, char* tGroupName, char* tVariableName);
int isMugenDefFloatVariableAsGroup(MugenDefScriptGroup* tGroup, char* tVariableName);
double getMugenDefFloatVariableAsGroup(MugenDefScriptGroup* tGroup, char* tVariableName);
int isMugenDefFloatVariableAsElement(MugenDefScriptGroupElement* tElement);
double getMugenDefFloatVariableAsElement(MugenDefScriptGroupElement* tElement);


int isMugenDefNumberVariable(MugenDefScript* tScript, char* tGroupName, char* tVariableName);
int getMugenDefNumberVariable(MugenDefScript* tScript, char* tGroupName, char* tVariableName);
int isMugenDefNumberVariableAsGroup(MugenDefScriptGroup* tGroup, char* tVariableName);
int getMugenDefNumberVariableAsGroup(MugenDefScriptGroup* tGroup, char* tVariableName);
int isMugenDefNumberVariableAsElement(MugenDefScriptGroupElement* tElement);
int getMugenDefNumberVariableAsElement(MugenDefScriptGroupElement* tElement);

int isMugenDefVectorVariable(MugenDefScript* tScript, char* tGroupName, char* tVariableName);
Vector3D getMugenDefVectorVariable(MugenDefScript* tScript, char* tGroupName, char* tVariableName);
int isMugenDefVectorVariableAsGroup(MugenDefScriptGroup* tGroup, char* tVariableName);
Vector3D getMugenDefVectorVariableAsGroup(MugenDefScriptGroup* tGroup, char* tVariableName);
int isMugenDefVectorVariableAsElement(MugenDefScriptGroupElement* tElement);
Vector3D getMugenDefVectorVariableAsElement(MugenDefScriptGroupElement* tElement);

int isMugenDefVectorIVariable(MugenDefScript* tScript, char* tGroupName, char* tVariableName);
int isMugenDefVectorIVariableAsGroup(MugenDefScriptGroup* tGroup, char* tVariableName);
Vector3DI getMugenDefVectorIVariableAsGroup(MugenDefScriptGroup* tGroup, char* tVariableName);
Vector3DI getMugenDefVectorIVariable(MugenDefScript* tScript, char* tGroupName, char* tVariableName);

int isMugenDefStringVectorVariableAsGroup(MugenDefScriptGroup* tGroup, char* tVariableName);
int isMugenDefStringVectorVariableAsElement(MugenDefScriptGroupElement* tElement);
MugenStringVector getMugenDefStringVectorVariable(MugenDefScript* tScript, char* tGroupName, char* tVariableName);
MugenStringVector getMugenDefStringVectorVariableAsGroup(MugenDefScriptGroup* tGroup, char* tVariableName);
MugenStringVector getMugenDefStringVectorVariableAsElement(MugenDefScriptGroupElement* tElement);
MugenStringVector copyMugenDefStringVectorVariableAsElement(MugenDefScriptGroupElement * tElement);

int isMugenDefGeoRectangleVariable(MugenDefScript* tScript, char* tGroupName, char* tVariableName);
int isMugenDefGeoRectangleVariableAsGroup(MugenDefScriptGroup* tGroup, char* tVariableName);
int isMugenDefGeoRectangleVariableAsElement(MugenDefScriptGroupElement * tElement);
GeoRectangle getMugenDefGeoRectangleVariable(MugenDefScript* tScript, char* tGroupName, char* tVariableName);
GeoRectangle getMugenDefGeoRectangleVariableAsGroup(MugenDefScriptGroup* tGroup, char* tVariableName);
GeoRectangle getMugenDefGeoRectangleVariableAsElement(MugenDefScriptGroupElement * tElement);

void getMugenDefStringOrDefault(char* tDst, MugenDefScript* s, char* tGroup, char* tVariable, char* tDefault);
char* getAllocatedMugenDefStringOrDefault(MugenDefScript* s, char* tGroup, char* tVariable, char* tDefault);
char* getAllocatedMugenDefStringOrDefaultAsGroup(MugenDefScriptGroup* tGroup, char* tVariable, char* tDefault);

double getMugenDefFloatOrDefault(MugenDefScript* s, char* tGroup, char* tVariable, double tDefault);
double getMugenDefFloatOrDefaultAsGroup(MugenDefScriptGroup* tGroup, char* tVariable, double tDefault);
int getMugenDefIntegerOrDefault(MugenDefScript* s, char* tGroup, char* tVariable, int tDefault);
int getMugenDefIntegerOrDefaultAsGroup(MugenDefScriptGroup* tGroup, char* tVariable, int tDefault);
Vector3D getMugenDefVectorOrDefault(MugenDefScript* s, char* tGroup, char* tVariable, Vector3D tDefault);
Vector3D getMugenDefVectorOrDefaultAsGroup(MugenDefScriptGroup* tGroup, char* tVariable, Vector3D tDefault);
Vector3DI getMugenDefVectorIOrDefault(MugenDefScript* s, char* tGroup, char* tVariable, Vector3DI tDefault);
Vector3DI getMugenDefVectorIOrDefaultAsGroup(MugenDefScriptGroup* tGroup, char* tVariable, Vector3DI tDefault);

GeoRectangle getMugenDefGeoRectangleOrDefault(MugenDefScript* s, char* tGroup, char* tVariable, GeoRectangle tDefault);
GeoRectangle getMugenDefGeoRectangleOrDefaultAsGroup(MugenDefScriptGroup* tGroup, char* tVariable, GeoRectangle tDefault);