#pragma once

#include "geometry.h"

#include "mugendefreader.h"

typedef enum {
	MUGEN_ASSIGNMENT_TYPE_FIXED_BOOLEAN,
	MUGEN_ASSIGNMENT_TYPE_AND,
	MUGEN_ASSIGNMENT_TYPE_OR,
	MUGEN_ASSIGNMENT_TYPE_COMPARISON,
	MUGEN_ASSIGNMENT_TYPE_INEQUALITY,
	MUGEN_ASSIGNMENT_TYPE_LESS_OR_EQUAL,
	MUGEN_ASSIGNMENT_TYPE_GREATER_OR_EQUAL,
	MUGEN_ASSIGNMENT_TYPE_VECTOR,
	MUGEN_ASSIGNMENT_TYPE_RANGE,
	MUGEN_ASSIGNMENT_TYPE_NULL,
	MUGEN_ASSIGNMENT_TYPE_NEGATION,
	MUGEN_ASSIGNMENT_TYPE_VARIABLE,
	MUGEN_ASSIGNMENT_TYPE_NUMBER,
	MUGEN_ASSIGNMENT_TYPE_FLOAT,
	MUGEN_ASSIGNMENT_TYPE_STRING,
	MUGEN_ASSIGNMENT_TYPE_ARRAY,
	MUGEN_ASSIGNMENT_TYPE_LESS,
	MUGEN_ASSIGNMENT_TYPE_GREATER,
	MUGEN_ASSIGNMENT_TYPE_ADDITION,
	MUGEN_ASSIGNMENT_TYPE_MULTIPLICATION,
	MUGEN_ASSIGNMENT_TYPE_MODULO,
	MUGEN_ASSIGNMENT_TYPE_SUBTRACTION,
	// MUGEN_ASSIGNMENT_TYPE_SET_VARIABLE, // TODO
	MUGEN_ASSIGNMENT_TYPE_DIVISION,
	MUGEN_ASSIGNMENT_TYPE_EXPONENTIATION,
	MUGEN_ASSIGNMENT_TYPE_UNARY_MINUS,
	MUGEN_ASSIGNMENT_TYPE_OPERATOR_ARGUMENT,
	MUGEN_ASSIGNMENT_TYPE_BITWISE_AND,
	MUGEN_ASSIGNMENT_TYPE_BITWISE_OR,
} MugenAssignmentType;

typedef struct {
	MugenAssignmentType mType;
	void* mData;

} MugenAssignment;

typedef struct {
	int mValue;
} MugenNumberAssignment;

typedef struct {
	double mValue;
} MugenFloatAssignment;

typedef struct {
	char* mValue;
} MugenStringAssignment;

typedef struct {
	char mName[100];
} MugenVariableAssignment;

typedef struct {
	int mValue;
} MugenFixedBooleanAssignment;

typedef struct {
	int mExcludeLeft;
	int mExcludeRight;
	MugenAssignment* a;
} MugenRangeAssignment;

typedef struct {
	MugenAssignment* a;
	MugenAssignment* b;
} MugenDependOnTwoAssignment;

typedef struct {
	MugenAssignment* a;
} MugenDependOnOneAssignment;

MugenAssignment* makeTrueMugenAssignment();

MugenAssignment* makeFalseMugenAssignment();
void destroyFalseMugenAssignment(MugenAssignment* tAssignment);
void destroyMugenAssignment(MugenAssignment* tAssignment);

MugenAssignment* makeNumberMugenAssignment(int tVal);
MugenAssignment * makeFloatMugenAssignment(double tVal);
MugenAssignment * makeStringMugenAssignment(char* tVal);
MugenAssignment* make2DVectorMugenAssignment(Vector3D tVal);
MugenAssignment* makeAndMugenAssignment(MugenAssignment* a, MugenAssignment* b);
MugenAssignment* makeOrMugenAssignment(MugenAssignment* a, MugenAssignment* b);

MugenAssignment*  parseMugenAssignmentFromString(char* tText);

int doMugenAssignmentStringsBeginsWithPattern(char* tPattern, char* tText);
int fetchMugenAssignmentFromGroupAndReturnWhetherItExists(char* tName, MugenDefScriptGroup* tGroup, MugenAssignment** tOutput);
void fetchMugenAssignmentFromGroupAndReturnWhetherItExistsDefaultString(char* tName, MugenDefScriptGroup* tGroup, MugenAssignment** tDst, char* tDefault);