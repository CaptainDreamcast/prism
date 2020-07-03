#pragma once

#include <string>

#include "datastructures.h"
#include "geometry.h"
#include "file.h"
#include "stlutil.h"

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
	std::string mName;
	MugenDefScriptGroupElementType mType;
	void* mData;

} MugenDefScriptGroupElement;

typedef struct MugenDefScriptGroup_t{
	std::string mName;
	std::map<std::string, MugenDefScriptGroupElement> mElements;
	List mOrderedElementList;
	struct MugenDefScriptGroup_t* mNext;
} MugenDefScriptGroup;

struct MugenDefScript {
	MugenDefScriptGroup* mFirstGroup;
	std::map<std::string, MugenDefScriptGroup> mGroups;
};

void loadMugenDefScript(MugenDefScript* oScript, const std::string& tPath);
void loadMugenDefScript(MugenDefScript* oScript, const char* tPath);
void loadMugenDefScriptFromBufferAndFreeBuffer(MugenDefScript* oScript, Buffer& tBuffer);
void unloadMugenDefScript(MugenDefScript* tScript);

int hasMugenDefScriptGroup(MugenDefScript* tScript, const char* tGroupName);
MugenDefScriptGroup* getMugenDefScriptGroup(MugenDefScript* tScript, const char* tGroupName);

MugenStringVector createAllocatedMugenStringVectorFromString(const char* tString);
void destroyMugenStringVector(MugenStringVector& tStringVector);

int isMugenDefStringVariable(MugenDefScript* tScript, const char* tGroupName, const char* tVariableName);
char* getAllocatedMugenDefStringVariable(MugenDefScript* tScript, const char* tGroupName, const char* tVariableName);
std::string getSTLMugenDefStringVariable(MugenDefScript* tScript, const char* tGroupName, const char* tVariableName);
int isMugenDefStringVariableAsGroup(MugenDefScriptGroup* tGroup, const char* tVariableName);
char* getAllocatedMugenDefStringVariableAsGroup(MugenDefScriptGroup* tGroup, const char* tVariableName);
std::string getSTLMugenDefStringVariableAsGroup(MugenDefScriptGroup* tGroup, const char* tVariableName);
int isMugenDefStringVariableAsElement(MugenDefScriptGroupElement* tElement);
char* getAllocatedMugenDefStringVariableForAssignmentAsElement(MugenDefScriptGroupElement* tElement);
std::string getSTLMugenDefStringVariableForAssignmentAsElement(MugenDefScriptGroupElement* tElement);
char* getAllocatedMugenDefStringVariableAsElement(MugenDefScriptGroupElement* tElement);
std::string getSTLMugenDefStringVariableAsElement(MugenDefScriptGroupElement* tElement);

int isMugenDefVariable(MugenDefScript* tScript, const char* tGroupName, const char* tVariableName);

int isMugenDefFloatVariable(MugenDefScript* tScript, const char* tGroupName, const char* tVariableName);
double getMugenDefFloatVariable(MugenDefScript* tScript, const char* tGroupName, const char* tVariableName);
int isMugenDefFloatVariableAsGroup(MugenDefScriptGroup* tGroup, const char* tVariableName);
double getMugenDefFloatVariableAsGroup(MugenDefScriptGroup* tGroup, const char* tVariableName);
int isMugenDefFloatVariableAsElement(MugenDefScriptGroupElement* tElement);
double getMugenDefFloatVariableAsElement(MugenDefScriptGroupElement* tElement);

int isMugenDefNumberVariable(MugenDefScript* tScript, const char* tGroupName, const char* tVariableName);
int getMugenDefNumberVariable(MugenDefScript* tScript, const char* tGroupName, const char* tVariableName);
int isMugenDefNumberVariableAsGroup(MugenDefScriptGroup* tGroup, const char* tVariableName);
int getMugenDefNumberVariableAsGroup(MugenDefScriptGroup* tGroup, const char* tVariableName);
int isMugenDefNumberVariableAsElement(MugenDefScriptGroupElement* tElement);
int getMugenDefNumberVariableAsElement(MugenDefScriptGroupElement* tElement);

int isMugenDefVectorVariable(MugenDefScript* tScript, const char* tGroupName, const char* tVariableName);
Vector3D getMugenDefVectorVariable(MugenDefScript* tScript, const char* tGroupName, const char* tVariableName);
int isMugenDefVectorVariableAsGroup(MugenDefScriptGroup* tGroup, const char* tVariableName);
Vector3D getMugenDefVectorVariableAsGroup(MugenDefScriptGroup* tGroup, const char* tVariableName);
int isMugenDefVectorVariableAsElement(MugenDefScriptGroupElement* tElement);
Vector3D getMugenDefVectorVariableAsElement(MugenDefScriptGroupElement* tElement);

Vector2D getMugenDefVector2DVariable(MugenDefScript* tScript, const char* tGroupName, const char* tVariableName);
Vector2D getMugenDefVector2DVariableAsGroup(MugenDefScriptGroup* tGroup, const char* tVariableName);
Vector2D getMugenDefVector2DVariableAsElement(MugenDefScriptGroupElement* tElement);

int isMugenDefVectorIVariable(MugenDefScript* tScript, const char* tGroupName, const char* tVariableName);
int isMugenDefVectorIVariableAsGroup(MugenDefScriptGroup* tGroup, const char* tVariableName);
Vector3DI getMugenDefVectorIVariableAsGroup(MugenDefScriptGroup* tGroup, const char* tVariableName);
Vector3DI getMugenDefVectorIVariable(MugenDefScript* tScript, const char* tGroupName, const char* tVariableName);

int isMugenDefVector2DIVariable(MugenDefScript* tScript, const char* tGroupName, const char* tVariableName);
int isMugenDefVector2DIVariableAsGroup(MugenDefScriptGroup* tGroup, const char* tVariableName);
Vector2DI getMugenDefVector2DIVariableAsGroup(MugenDefScriptGroup* tGroup, const char* tVariableName);
Vector2DI getMugenDefVector2DIVariable(MugenDefScript* tScript, const char* tGroupName, const char* tVariableName);

int isMugenDefStringVectorVariableAsGroup(MugenDefScriptGroup* tGroup, const char* tVariableName);
int isMugenDefStringVectorVariableAsElement(MugenDefScriptGroupElement* tElement);
MugenStringVector getMugenDefStringVectorVariable(MugenDefScript* tScript, const char* tGroupName, const char* tVariableName);
MugenStringVector getMugenDefStringVectorVariableAsGroup(MugenDefScriptGroup* tGroup, const char* tVariableName);
MugenStringVector getMugenDefStringVectorVariableAsElement(MugenDefScriptGroupElement* tElement);
MugenStringVector copyMugenDefStringVectorVariableAsElement(MugenDefScriptGroupElement * tElement);

int isMugenDefGeoRectangle2DVariable(MugenDefScript* tScript, const char* tGroupName, const char* tVariableName);
int isMugenDefGeoRectangle2DVariableAsGroup(MugenDefScriptGroup* tGroup, const char* tVariableName);
int isMugenDefGeoRectangle2DVariableAsElement(MugenDefScriptGroupElement * tElement);
GeoRectangle2D getMugenDefGeoRectangle2DVariable(MugenDefScript* tScript, const char* tGroupName, const char* tVariableName);
GeoRectangle2D getMugenDefGeoRectangle2DVariableAsGroup(MugenDefScriptGroup* tGroup, const char* tVariableName);
GeoRectangle2D getMugenDefGeoRectangle2DVariableAsElement(MugenDefScriptGroupElement * tElement);

void getMugenDefStringOrDefault(char* tDst, MugenDefScript* s, const char* tGroup, const char* tVariable, const char* tDefault);
char* getAllocatedMugenDefStringOrDefault(MugenDefScript* s, const char* tGroup, const char* tVariable, const char* tDefault);
char* getAllocatedMugenDefStringOrDefaultAsGroup(MugenDefScriptGroup* tGroup, const char* tVariable, const char* tDefault);
std::string getSTLMugenDefStringOrDefault(MugenDefScript* s, const char* tGroup, const char* tVariable, const char* tDefault);
std::string getSTLMugenDefStringOrDefaultAsGroup(MugenDefScriptGroup* tGroup, const char* tVariable, const char* tDefault);

double getMugenDefFloatOrDefault(MugenDefScript* s, const char* tGroup, const char* tVariable, double tDefault);
double getMugenDefFloatOrDefaultAsGroup(MugenDefScriptGroup* tGroup, const char* tVariable, double tDefault);
int getMugenDefIntegerOrDefault(MugenDefScript* s, const char* tGroup, const char* tVariable, int tDefault);
int getMugenDefIntegerOrDefaultAsGroup(MugenDefScriptGroup* tGroup, const char* tVariable, int tDefault);
Vector3D getMugenDefVectorOrDefault(MugenDefScript* s, const char* tGroup, const char* tVariable, const Vector3D& tDefault);
Vector3D getMugenDefVectorOrDefaultAsGroup(MugenDefScriptGroup* tGroup, const char* tVariable, const Vector3D& tDefault);
Vector2D getMugenDefVector2DOrDefault(MugenDefScript* s, const char* tGroup, const char* tVariable, const Vector2D& tDefault);
Vector2D getMugenDefVector2DOrDefaultAsGroup(MugenDefScriptGroup* tGroup, const char* tVariable, const Vector2D& tDefault);
Vector3DI getMugenDefVectorIOrDefault(MugenDefScript* s, const char* tGroup, const char* tVariable, const Vector3DI& tDefault);
Vector3DI getMugenDefVectorIOrDefaultAsGroup(MugenDefScriptGroup* tGroup, const char* tVariable, const Vector3DI& tDefault);
Vector2DI getMugenDefVector2DIOrDefault(MugenDefScript* s, const char* tGroup, const char* tVariable, const Vector2DI& tDefault);
Vector2DI getMugenDefVector2DIOrDefaultAsGroup(MugenDefScriptGroup* tGroup, const char* tVariable, const Vector2DI& tDefault);

GeoRectangle2D getMugenDefGeoRectangle2DOrDefault(MugenDefScript* s, const char* tGroup, const char* tVariable, const GeoRectangle2D& tDefault);
GeoRectangle2D getMugenDefGeoRectangle2DOrDefaultAsGroup(MugenDefScriptGroup* tGroup, const char* tVariable, const GeoRectangle2D& tDefault);