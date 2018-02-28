#pragma once

#include "mugenassignment.h"

int evaluateMugenAssignment(MugenAssignment* tAssignment, void* tCaller);
double evaluateMugenAssignmentAndReturnAsFloat(MugenAssignment* tAssignment, void* tCaller);
int evaluateMugenAssignmentAndReturnAsInteger(MugenAssignment* tAssignment, void* tCaller);
char* evaluateMugenAssignmentAndReturnAsAllocatedString(MugenAssignment* tAssignment, void* tCaller);
Vector3D evaluateMugenAssignmentAndReturnAsVector3D(MugenAssignment* tAssignment, void* tCaller);

void resetMugenAssignmentContext();
void addMugenAssignmentVariable(char* tVariable, void(*tEvalFunc)(char* tOutput, void* tCaller));
void addMugenAssignmentArray(char* tVariable, void(*tEvalFunc)(char* tOutput, void* tCaller, char* tArrayIndex));


int getMugenAssignmentAsIntegerValueOrDefaultWhenEmpty(MugenAssignment* tAssignment, void* tCaller, int tDefault);
double getMugenAssignmentAsFloatValueOrDefaultWhenEmpty(MugenAssignment* tAssignment, void* tCaller, double tDefault);
Vector3D getMugenAssignmentAsVector3DValueOrDefaultWhenEmpty(MugenAssignment* tAssignment, void* tCaller, Vector3D tDefault);
