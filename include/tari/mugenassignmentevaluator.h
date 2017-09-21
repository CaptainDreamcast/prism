#pragma once

#include "mugenassignment.h"

fup int evaluateMugenAssignment(MugenAssignment* tAssignment, void* tCaller);
fup double evaluateMugenAssignmentAndReturnAsFloat(MugenAssignment* tAssignment, void* tCaller);
fup int evaluateMugenAssignmentAndReturnAsInteger(MugenAssignment* tAssignment, void* tCaller);
fup char* evaluateMugenAssignmentAndReturnAsAllocatedString(MugenAssignment* tAssignment, void* tCaller);
fup Vector3D evaluateMugenAssignmentAndReturnAsVector3D(MugenAssignment* tAssignment, void* tCaller);

fup void resetMugenAssignmentContext();
fup void addMugenAssignmentVariable(char* tVariable, void(*tEvalFunc)(char* tOutput, void* tCaller));
fup void addMugenAssignmentArray(char* tVariable, void(*tEvalFunc)(char* tOutput, void* tCaller, char* tArrayIndex));


fup int getMugenAssignmentAsIntegerValueOrDefaultWhenEmpty(MugenAssignment* tAssignment, void* tCaller, int tDefault);
fup double getMugenAssignmentAsFloatValueOrDefaultWhenEmpty(MugenAssignment* tAssignment, void* tCaller, double tDefault);
fup Vector3D getMugenAssignmentAsVector3DValueOrDefaultWhenEmpty(MugenAssignment* tAssignment, void* tCaller, Vector3D tDefault);
