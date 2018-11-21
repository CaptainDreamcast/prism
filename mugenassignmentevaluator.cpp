#include "prism/mugenassignmentevaluator.h"

#include <assert.h>
#include <string.h>

#include "prism/log.h"
#include "prism/system.h"
#include "prism/math.h"
#include "prism/memoryhandler.h"

typedef struct {
	void(*mEvalFunc)(char* tOutput, void* tCaller);
} MugenAssignmentVariableEntry;

typedef struct {
	void(*mEvalFunc)(char* tOutput, void* tCaller, char* tArrayIndex);
} MugenAssignmentArrayEntry;

static struct {
	StringMap mVariables;
	StringMap mArrays;

} gData;

typedef struct {
	char mValue[100];
} AssignmentReturnValue;

static AssignmentReturnValue evaluateAssignmentInternal(MugenAssignment* tAssignment, void* tCaller);

static AssignmentReturnValue getAssignmentReturnValueToBool(AssignmentReturnValue tAssignmentReturn) {
	AssignmentReturnValue ret;
	if (!strcmp("", tAssignmentReturn.mValue) || !strcmp("0", tAssignmentReturn.mValue)) {
		strcpy(ret.mValue, "0");
	}
	else {
		strcpy(ret.mValue, "1");
	}

	return ret;
}

static int evaluateAssignmentReturnAsBool(AssignmentReturnValue tAssignmentReturn) {
	AssignmentReturnValue rest = getAssignmentReturnValueToBool(tAssignmentReturn);

	return strcmp("0", rest.mValue);
}


static int evaluateAssignmentReturnAsNumber(AssignmentReturnValue tAssignmentReturn) {
	return atoi(tAssignmentReturn.mValue);
}

static double evaluateAssignmentReturnAsFloat(AssignmentReturnValue tAssignmentReturn) {
	return atof(tAssignmentReturn.mValue);
}


static AssignmentReturnValue makeBooleanAssignmentReturn(int tValue) {
	AssignmentReturnValue ret;
	if (tValue) {
		strcpy(ret.mValue, "1");
	}
	else {
		strcpy(ret.mValue, "0");
	}
	return ret;
}

static AssignmentReturnValue makeNumberAssignmentReturn(int tValue) {
	AssignmentReturnValue ret;
	sprintf(ret.mValue, "%d", tValue);
	return ret;
}

static AssignmentReturnValue makeFloatAssignmentReturn(double tValue) {
	AssignmentReturnValue ret;
	sprintf(ret.mValue, "%f", tValue);
	return ret;
}

static AssignmentReturnValue makeStringAssignmentReturn(char* tValue) {
	AssignmentReturnValue ret;
	strcpy(ret.mValue, tValue);
	return ret;
}

static AssignmentReturnValue evaluateOrAssignment(MugenAssignment* tAssignment, void* tCaller) {
	MugenDependOnTwoAssignment* orAssignment = tAssignment->mData;

	AssignmentReturnValue a = evaluateAssignmentInternal(orAssignment->a, tCaller);
	int valA = evaluateAssignmentReturnAsBool(a);
	if (valA) return makeBooleanAssignmentReturn(valA);

	AssignmentReturnValue b = evaluateAssignmentInternal(orAssignment->b, tCaller);
	int valB = evaluateAssignmentReturnAsBool(b);

	return makeBooleanAssignmentReturn(valB);
}

static AssignmentReturnValue evaluateAndAssignment(MugenAssignment* tAssignment, void* tCaller) {
	MugenDependOnTwoAssignment* andAssignment = tAssignment->mData;

	AssignmentReturnValue a = evaluateAssignmentInternal(andAssignment->a, tCaller);
	int valA = evaluateAssignmentReturnAsBool(a);
	if (!valA) return makeBooleanAssignmentReturn(valA);

	AssignmentReturnValue b = evaluateAssignmentInternal(andAssignment->b, tCaller);
	int valB = evaluateAssignmentReturnAsBool(b);

	return makeBooleanAssignmentReturn(valB);
}

static AssignmentReturnValue evaluateBitwiseOrAssignment(MugenAssignment* tAssignment, void* tCaller) {
	MugenDependOnTwoAssignment* bitwiseOrAssignment = tAssignment->mData;

	AssignmentReturnValue a = evaluateAssignmentInternal(bitwiseOrAssignment->a, tCaller);
	int valA = evaluateAssignmentReturnAsNumber(a);

	AssignmentReturnValue b = evaluateAssignmentInternal(bitwiseOrAssignment->b, tCaller);
	int valB = evaluateAssignmentReturnAsNumber(b);

	return makeNumberAssignmentReturn(valA | valB);
}

static AssignmentReturnValue evaluateBitwiseAndAssignment(MugenAssignment* tAssignment, void* tCaller) {
	MugenDependOnTwoAssignment* bitwiseAndAssignment = tAssignment->mData;

	AssignmentReturnValue a = evaluateAssignmentInternal(bitwiseAndAssignment->a, tCaller);
	int valA = evaluateAssignmentReturnAsNumber(a);

	AssignmentReturnValue b = evaluateAssignmentInternal(bitwiseAndAssignment->b, tCaller);
	int valB = evaluateAssignmentReturnAsNumber(b);

	return makeNumberAssignmentReturn(valA & valB);
}

static int isRangeAssignmentReturn(AssignmentReturnValue ret) {
	char brace[100];
	sscanf(ret.mValue, "%s", brace);
	return !strcmp("[", brace);
}

static AssignmentReturnValue evaluateRangeComparisonAssignment(AssignmentReturnValue a, AssignmentReturnValue tRange) {
	int val = evaluateAssignmentReturnAsNumber(a);

	char openBrace[10], valString1[20], comma[10], valString2[20], closeBrace[10];
	sscanf(tRange.mValue, "%s %s %s %s %s", openBrace, valString1, comma, valString2, closeBrace);
	assert(!strcmp("[", openBrace));
	assert(strcmp("", valString1));
	assert(!strcmp(",", comma));
	assert(strcmp("", valString1));
	assert(!strcmp("]", closeBrace));

	int val1 = atoi(valString1);
	int val2 = atoi(valString2);

	return makeBooleanAssignmentReturn(val >= val1 && val <= val2);
}

static int isFloatReturn(AssignmentReturnValue tReturn) {
	if (strchr(tReturn.mValue, '.') == NULL) return 0;

	char* text = tReturn.mValue;
	int n = strlen(text);

	int i;
	for (i = 0; i < n; i++) {
		if (text[i] != '.' && (text[i] < '0' || text[i] > '9')) return 0;
	}

	return 1;
}


static AssignmentReturnValue evaluateComparisonAssignmentInternal(MugenAssignment* mAssignment, AssignmentReturnValue b, void* tCaller) {
	char name[MUGEN_DEF_STRING_LENGTH];
	
	if (mAssignment->mType == MUGEN_ASSIGNMENT_TYPE_VARIABLE) {
		MugenVariableAssignment* var = mAssignment->mData;
		strcpy(name, var->mName);
		turnStringLowercase(name);
	}
	else {
		name[0] = '\0';
	}

	AssignmentReturnValue a = evaluateAssignmentInternal(mAssignment, tCaller);

	// TODO special comparisons
	(void)name;

	
	if (isRangeAssignmentReturn(b)) {
		return evaluateRangeComparisonAssignment(a, b);
	}
	else if (isFloatReturn(a) || isFloatReturn(b)) {
		int value = evaluateAssignmentReturnAsFloat(a) == evaluateAssignmentReturnAsFloat(b);
		return makeBooleanAssignmentReturn(value);
	}
	else {
		int value = !strcmp(a.mValue, b.mValue);
		return makeBooleanAssignmentReturn(value);
	}
}

static int isCallerRedirectAssignment(AssignmentReturnValue a, void* tCaller) {
	(void)a;
	(void)tCaller;

	return 0; // TODO
}

static void* getCallerRedirectAssignment(AssignmentReturnValue a, void* tCaller) {
	(void)a;
	(void)tCaller;

	return NULL; // TODO
}

static AssignmentReturnValue evaluateComparisonAssignment(MugenAssignment* tAssignment, void* tCaller) {
	MugenDependOnTwoAssignment* comparisonAssignment = tAssignment->mData;
	//printf("eval assign a\n");

	AssignmentReturnValue b = evaluateAssignmentInternal(comparisonAssignment->b, tCaller);

	if (comparisonAssignment->a->mType == MUGEN_ASSIGNMENT_TYPE_VECTOR) {
		MugenDependOnTwoAssignment* vectorAssignment = comparisonAssignment->a->mData;
		AssignmentReturnValue vecA = evaluateAssignmentInternal(vectorAssignment->a, tCaller);

		if (isCallerRedirectAssignment(vecA, tCaller)) {
			void* target = getCallerRedirectAssignment(vecA, tCaller);
			assert(target);
			return evaluateComparisonAssignmentInternal(vectorAssignment->b, b, target);
		}
	}

	return evaluateComparisonAssignmentInternal(comparisonAssignment->a, b, tCaller);
}

static AssignmentReturnValue evaluateInequalityAssignment(MugenAssignment* tAssignment, void* tCaller) {
	AssignmentReturnValue equal = evaluateComparisonAssignment(tAssignment, tCaller);
	int val = evaluateAssignmentReturnAsBool(equal);

	return makeBooleanAssignmentReturn(!val);
}

static AssignmentReturnValue evaluateGreaterIntegers(AssignmentReturnValue a, AssignmentReturnValue b) {
	int val = evaluateAssignmentReturnAsNumber(a) > evaluateAssignmentReturnAsNumber(b);
	return makeBooleanAssignmentReturn(val);
}

static AssignmentReturnValue evaluateGreaterFloats(AssignmentReturnValue a, AssignmentReturnValue b) {
	int val = evaluateAssignmentReturnAsFloat(a) > evaluateAssignmentReturnAsFloat(b);
	return makeBooleanAssignmentReturn(val);
}

static AssignmentReturnValue evaluateGreaterAssignment(MugenAssignment* tAssignment, void* tCaller) {
	MugenDependOnTwoAssignment* greaterAssignment = tAssignment->mData;
	AssignmentReturnValue a = evaluateAssignmentInternal(greaterAssignment->a, tCaller);
	AssignmentReturnValue b = evaluateAssignmentInternal(greaterAssignment->b, tCaller);

	if (isFloatReturn(a) || isFloatReturn(b)) {
		return evaluateGreaterFloats(a, b);
	}
	else {
		return evaluateGreaterIntegers(a, b);
	}
}

static AssignmentReturnValue evaluateGreaterOrEqualIntegers(AssignmentReturnValue a, AssignmentReturnValue b) {
	int val = evaluateAssignmentReturnAsNumber(a) >= evaluateAssignmentReturnAsNumber(b);
	return makeBooleanAssignmentReturn(val);
}

static AssignmentReturnValue evaluateGreaterOrEqualFloats(AssignmentReturnValue a, AssignmentReturnValue b) {
	int val = evaluateAssignmentReturnAsFloat(a) >= evaluateAssignmentReturnAsFloat(b);
	return makeBooleanAssignmentReturn(val);
}

static AssignmentReturnValue evaluateGreaterOrEqualAssignment(MugenAssignment* tAssignment, void* tCaller) {
	MugenDependOnTwoAssignment* greaterOrEqualAssignment = tAssignment->mData;
	AssignmentReturnValue a = evaluateAssignmentInternal(greaterOrEqualAssignment->a, tCaller);
	AssignmentReturnValue b = evaluateAssignmentInternal(greaterOrEqualAssignment->b, tCaller);

	if (isFloatReturn(a) || isFloatReturn(b)) {
		return evaluateGreaterOrEqualFloats(a, b);
	}
	else {
		return evaluateGreaterOrEqualIntegers(a, b);
	}
}

static AssignmentReturnValue evaluateLessIntegers(AssignmentReturnValue a, AssignmentReturnValue b) {
	int val = evaluateAssignmentReturnAsNumber(a) < evaluateAssignmentReturnAsNumber(b);
	return makeBooleanAssignmentReturn(val);
}

static AssignmentReturnValue evaluateLessFloats(AssignmentReturnValue a, AssignmentReturnValue b) {
	int val = evaluateAssignmentReturnAsFloat(a) < evaluateAssignmentReturnAsFloat(b);
	return makeBooleanAssignmentReturn(val);
}

static AssignmentReturnValue evaluateLessAssignment(MugenAssignment* tAssignment, void* tCaller) {
	MugenDependOnTwoAssignment* lessAssignment = tAssignment->mData;
	AssignmentReturnValue a = evaluateAssignmentInternal(lessAssignment->a, tCaller);
	AssignmentReturnValue b = evaluateAssignmentInternal(lessAssignment->b, tCaller);

	if (isFloatReturn(a) || isFloatReturn(b)) {
		return evaluateLessFloats(a, b);
	}
	else {
		return evaluateLessIntegers(a, b);
	}
}

static AssignmentReturnValue evaluateLessOrEqualIntegers(AssignmentReturnValue a, AssignmentReturnValue b) {
	int val = evaluateAssignmentReturnAsNumber(a) <= evaluateAssignmentReturnAsNumber(b);
	return makeBooleanAssignmentReturn(val);
}

static AssignmentReturnValue evaluateLessOrEqualFloats(AssignmentReturnValue a, AssignmentReturnValue b) {
	int val = evaluateAssignmentReturnAsFloat(a) <= evaluateAssignmentReturnAsFloat(b);
	return makeBooleanAssignmentReturn(val);
}

static AssignmentReturnValue evaluateLessOrEqualAssignment(MugenAssignment* tAssignment, void* tCaller) {
	MugenDependOnTwoAssignment* lessOrEqualAssignment = tAssignment->mData;
	AssignmentReturnValue a = evaluateAssignmentInternal(lessOrEqualAssignment->a, tCaller);
	AssignmentReturnValue b = evaluateAssignmentInternal(lessOrEqualAssignment->b, tCaller);

	if (isFloatReturn(a) || isFloatReturn(b)) {
		return evaluateLessOrEqualFloats(a, b);
	}
	else {
		return evaluateLessOrEqualIntegers(a, b);
	}
}

static AssignmentReturnValue evaluateModuloIntegers(AssignmentReturnValue a, AssignmentReturnValue b) {
	int val = evaluateAssignmentReturnAsNumber(a) % evaluateAssignmentReturnAsNumber(b);
	return makeNumberAssignmentReturn(val);
}

static AssignmentReturnValue evaluateModuloFloats(AssignmentReturnValue a, AssignmentReturnValue b) {
	double valA = evaluateAssignmentReturnAsFloat(a);
	double valB = evaluateAssignmentReturnAsFloat(b);
	return makeFloatAssignmentReturn(fmod(valA, valB));
}

static AssignmentReturnValue evaluateModuloAssignment(MugenAssignment* tAssignment, void* tCaller) {
	MugenDependOnTwoAssignment* moduloAssignment = tAssignment->mData;
	AssignmentReturnValue a = evaluateAssignmentInternal(moduloAssignment->a, tCaller);
	AssignmentReturnValue b = evaluateAssignmentInternal(moduloAssignment->b, tCaller);

	if (isFloatReturn(a) || isFloatReturn(b)) {
		return evaluateModuloFloats(a, b);
	}
	else {
		return evaluateModuloIntegers(a, b);
	}
}

static int powI(int a, int b) {
	assert(b >= 0);

	if (a == 0 && b == 0) return 1;

	int ret = 1;
	while (b--) ret *= a;

	return ret;
}

static AssignmentReturnValue evaluateExponentiationIntegers(AssignmentReturnValue a, AssignmentReturnValue b) {
	int val1 = evaluateAssignmentReturnAsNumber(a);
	int val2 = evaluateAssignmentReturnAsNumber(b);
	return makeNumberAssignmentReturn(powI(val1, val2));
}

static AssignmentReturnValue evaluateExponentiationFloats(AssignmentReturnValue a, AssignmentReturnValue b) {
	double val1 = evaluateAssignmentReturnAsFloat(a);
	double val2 = evaluateAssignmentReturnAsFloat(b);
	return makeFloatAssignmentReturn(pow(val1, val2));
}

static AssignmentReturnValue evaluateExponentiationAssignment(MugenAssignment* tAssignment, void* tCaller) {
	MugenDependOnTwoAssignment* exponentiationAssignment = tAssignment->mData;
	AssignmentReturnValue a = evaluateAssignmentInternal(exponentiationAssignment->a, tCaller);
	AssignmentReturnValue b = evaluateAssignmentInternal(exponentiationAssignment->b, tCaller);

	if (isFloatReturn(a) || isFloatReturn(b) || evaluateAssignmentReturnAsNumber(b) < 0) {
		return evaluateExponentiationFloats(a, b);
	}
	else {
		return evaluateExponentiationIntegers(a, b);
	}
}


static AssignmentReturnValue evaluateMultiplicationIntegers(AssignmentReturnValue a, AssignmentReturnValue b) {
	int val = evaluateAssignmentReturnAsNumber(a) * evaluateAssignmentReturnAsNumber(b);
	return makeNumberAssignmentReturn(val);
}

static AssignmentReturnValue evaluateMultiplicationFloats(AssignmentReturnValue a, AssignmentReturnValue b) {
	double val = evaluateAssignmentReturnAsFloat(a) * evaluateAssignmentReturnAsFloat(b);
	return makeFloatAssignmentReturn(val);
}

static AssignmentReturnValue evaluateMultiplicationAssignment(MugenAssignment* tAssignment, void* tCaller) {
	MugenDependOnTwoAssignment* multiplicationAssignment = tAssignment->mData;
	AssignmentReturnValue a = evaluateAssignmentInternal(multiplicationAssignment->a, tCaller);
	AssignmentReturnValue b = evaluateAssignmentInternal(multiplicationAssignment->b, tCaller);

	if (isFloatReturn(a) || isFloatReturn(b)) {
		return evaluateMultiplicationFloats(a, b);
	}
	else {
		return evaluateMultiplicationIntegers(a, b);
	}
}

static AssignmentReturnValue evaluateDivisionIntegers(AssignmentReturnValue a, AssignmentReturnValue b) {
	int val = evaluateAssignmentReturnAsNumber(a) / evaluateAssignmentReturnAsNumber(b);
	return makeNumberAssignmentReturn(val);
}

static AssignmentReturnValue evaluateDivisionFloats(AssignmentReturnValue a, AssignmentReturnValue b) {
	double val = evaluateAssignmentReturnAsFloat(a) / evaluateAssignmentReturnAsFloat(b);
	return makeFloatAssignmentReturn(val);
}

static AssignmentReturnValue evaluateDivisionAssignment(MugenAssignment* tAssignment, void* tCaller) {
	MugenDependOnTwoAssignment* divisionAssignment = tAssignment->mData;
	AssignmentReturnValue a = evaluateAssignmentInternal(divisionAssignment->a, tCaller);
	AssignmentReturnValue b = evaluateAssignmentInternal(divisionAssignment->b, tCaller);

	if (isFloatReturn(a) || isFloatReturn(b)) {
		return evaluateDivisionFloats(a, b);
	}
	else {
		return evaluateDivisionIntegers(a, b);
	}
}

static int isSparkFileReturn(AssignmentReturnValue a) {
	char firstW[200];
	int items = sscanf(a.mValue, "%s", firstW);
	if (!items) return 0;

	return !strcmp("isinotherfile", firstW);
}

static AssignmentReturnValue evaluateAdditionSparkFile(AssignmentReturnValue a, AssignmentReturnValue b) {
	char firstW[200];
	int val1;

	int items = sscanf(a.mValue, "%s %d", firstW, &val1);
	assert(items == 2);

	int val2 = evaluateAssignmentReturnAsNumber(b);

	AssignmentReturnValue ret;
	sprintf(ret.mValue, "isinotherfile %d", val1+val2);

	return ret;
}


static AssignmentReturnValue evaluateAdditionIntegers(AssignmentReturnValue a, AssignmentReturnValue b) {
	int val = evaluateAssignmentReturnAsNumber(a) + evaluateAssignmentReturnAsNumber(b);
	return makeNumberAssignmentReturn(val);
}

static AssignmentReturnValue evaluateAdditionFloats(AssignmentReturnValue a, AssignmentReturnValue b) {
	double val = evaluateAssignmentReturnAsFloat(a) + evaluateAssignmentReturnAsFloat(b);
	return makeFloatAssignmentReturn(val);
}

static AssignmentReturnValue evaluateAdditionAssignment(MugenAssignment* tAssignment, void* tCaller) {
	MugenDependOnTwoAssignment* additionAssignment = tAssignment->mData;
	AssignmentReturnValue a = evaluateAssignmentInternal(additionAssignment->a, tCaller);
	AssignmentReturnValue b = evaluateAssignmentInternal(additionAssignment->b, tCaller);

	if (isSparkFileReturn(a)) {
		return evaluateAdditionSparkFile(a, b);
	}
	else if (isFloatReturn(a) || isFloatReturn(b)) {
		return evaluateAdditionFloats(a, b);
	}
	else {
		return evaluateAdditionIntegers(a, b);
	}
}


static AssignmentReturnValue evaluateSubtractionIntegers(AssignmentReturnValue a, AssignmentReturnValue b) {
	int val = evaluateAssignmentReturnAsNumber(a) - evaluateAssignmentReturnAsNumber(b);
	return makeNumberAssignmentReturn(val);
}

static AssignmentReturnValue evaluateSubtractionFloats(AssignmentReturnValue a, AssignmentReturnValue b) {
	double val = evaluateAssignmentReturnAsFloat(a) - evaluateAssignmentReturnAsFloat(b);
	return makeFloatAssignmentReturn(val);
}

static AssignmentReturnValue evaluateSubtractionAssignment(MugenAssignment* tAssignment, void* tCaller) {
	MugenDependOnTwoAssignment* subtractionAssignment = tAssignment->mData;
	AssignmentReturnValue a = evaluateAssignmentInternal(subtractionAssignment->a, tCaller);
	AssignmentReturnValue b = evaluateAssignmentInternal(subtractionAssignment->b, tCaller);

	if (isFloatReturn(a) || isFloatReturn(b)) {
		return evaluateSubtractionFloats(a, b);
	}
	else {
		return evaluateSubtractionIntegers(a, b);
	}
}


static AssignmentReturnValue evaluateOperatorArgumentAssignment(MugenAssignment* tAssignment, void* tCaller) {
	MugenDependOnTwoAssignment* operatorAssignment = tAssignment->mData;
	AssignmentReturnValue a = evaluateAssignmentInternal(operatorAssignment->a, tCaller);
	AssignmentReturnValue b = evaluateAssignmentInternal(operatorAssignment->b, tCaller);

	AssignmentReturnValue ret;
	sprintf(ret.mValue, "%s %s", a.mValue, b.mValue);
	return ret;
}

static AssignmentReturnValue evaluateBooleanAssignment(MugenAssignment* tAssignment) {
	MugenFixedBooleanAssignment* fixedAssignment = tAssignment->mData;

	return makeBooleanAssignmentReturn(fixedAssignment->mValue);
}

static AssignmentReturnValue evaluateNumberAssignment(MugenAssignment* tAssignment) {
	MugenNumberAssignment* number = tAssignment->mData;

	return makeNumberAssignmentReturn(number->mValue);
}

static AssignmentReturnValue evaluateFloatAssignment(MugenAssignment* tAssignment) {
	MugenFloatAssignment* f = tAssignment->mData;

	return makeFloatAssignmentReturn(f->mValue);
}

static AssignmentReturnValue evaluateStringAssignment(MugenAssignment* tAssignment) {
	MugenStringAssignment* s = tAssignment->mData;

	return makeStringAssignmentReturn(s->mValue);
}

static AssignmentReturnValue evaluateCallerRedirectVectorAssignment(AssignmentReturnValue tFirstValue, MugenDependOnTwoAssignment* tVectorAssignment, void* tCaller) {
	void* target = getCallerRedirectAssignment(tFirstValue, tCaller);
	assert(target != NULL);
	assert(tVectorAssignment->b->mType == MUGEN_ASSIGNMENT_TYPE_VARIABLE || tVectorAssignment->b->mType == MUGEN_ASSIGNMENT_TYPE_ARRAY);

	return evaluateAssignmentInternal(tVectorAssignment->b, target);
}

static AssignmentReturnValue evaluateVectorAssignment(MugenAssignment* tAssignment, void* tCaller) {
	MugenDependOnTwoAssignment* vectorAssignment = tAssignment->mData;
	AssignmentReturnValue a = evaluateAssignmentInternal(vectorAssignment->a, tCaller);

	if (isCallerRedirectAssignment(a, tCaller)) {
		return evaluateCallerRedirectVectorAssignment(a, vectorAssignment, tCaller);
	}
	else {
		AssignmentReturnValue b = evaluateAssignmentInternal(vectorAssignment->b, tCaller);

		AssignmentReturnValue ret;
		sprintf(ret.mValue, "%s , %s", a.mValue, b.mValue);
		return ret;
	}
}

static AssignmentReturnValue evaluateRangeAssignment(MugenAssignment* tAssignment, void* tCaller) {
	MugenRangeAssignment* rangeAssignment = tAssignment->mData;
	AssignmentReturnValue a = evaluateAssignmentInternal(rangeAssignment->a, tCaller);

	char valString1[100], comma[10], valString2[100];
	sscanf(a.mValue, "%s %s %s", valString1, comma, valString2);
	assert(strcmp("", valString1));
	assert(!strcmp(",", comma));
	assert(strcmp("", valString2));

	int val1 = atoi(valString1);
	int val2 = atoi(valString2);
	if (rangeAssignment->mExcludeLeft) val1++;
	if (rangeAssignment->mExcludeRight) val2--;

	AssignmentReturnValue ret;
	sprintf(ret.mValue, "[ %d , %d ]", val1, val2);
	return ret;
}


static AssignmentReturnValue evaluateVariableAssignment(MugenAssignment* tAssignment, void* tCaller) {
	MugenVariableAssignment* variable = tAssignment->mData;
	char testString[100];
	strcpy(testString, variable->mName);
	turnStringLowercase(testString);

	// TODO: strings beginning with letters
	if (testString[0] == 's' || testString[0] == 'f') {
		AssignmentReturnValue ret;
		sprintf(ret.mValue, "isinotherfile %s", testString + 1);
		return ret;
	}
	
	// TODO: strings beginning with words
	if (doMugenAssignmentStringsBeginsWithPattern("projhit", testString)) { 
		return makeBooleanAssignmentReturn(0);
	}


	if(!string_map_contains(&gData.mVariables, testString)) {
		logError("Unknown variable.");
		logErrorString(testString);
		recoverFromError();
		return makeBooleanAssignmentReturn(0);
	}

	MugenAssignmentVariableEntry* e = string_map_get(&gData.mVariables, testString);
	AssignmentReturnValue ret;
	e->mEvalFunc(ret.mValue, tCaller);
	return ret;
}


static AssignmentReturnValue evaluateAbsArrayAssignment(AssignmentReturnValue tIndex) {
	if (isFloatReturn(tIndex)) {
		double val = evaluateAssignmentReturnAsFloat(tIndex);
		return makeFloatAssignmentReturn(fabs(val));
	}
	else {
		int val = evaluateAssignmentReturnAsNumber(tIndex);
		return makeNumberAssignmentReturn(abs(val));
	}
}

static AssignmentReturnValue evaluateExpArrayAssignment(AssignmentReturnValue tIndex) {
		double val = evaluateAssignmentReturnAsFloat(tIndex);
		return makeFloatAssignmentReturn(exp(val));
}

static AssignmentReturnValue evaluateNaturalLogArrayAssignment(AssignmentReturnValue tIndex) {
	double val = evaluateAssignmentReturnAsFloat(tIndex);
	return makeFloatAssignmentReturn(log(val));
}

static AssignmentReturnValue evaluateCosineArrayAssignment(AssignmentReturnValue tIndex) {
		double val = evaluateAssignmentReturnAsFloat(tIndex);
		return makeFloatAssignmentReturn(cos(val));
}

static AssignmentReturnValue evaluateSineArrayAssignment(AssignmentReturnValue tIndex) {
	double val = evaluateAssignmentReturnAsFloat(tIndex);
	return makeFloatAssignmentReturn(sin(val));
}

static AssignmentReturnValue evaluateFloorArrayAssignment(AssignmentReturnValue tIndex) {
	if (isFloatReturn(tIndex)) {
		double val = evaluateAssignmentReturnAsFloat(tIndex);
		return makeNumberAssignmentReturn((int)floor(val));
	}
	else {
		int val = evaluateAssignmentReturnAsNumber(tIndex);
		return makeNumberAssignmentReturn(val);
	}
}

static AssignmentReturnValue evaluateCeilArrayAssignment(AssignmentReturnValue tIndex) {
	if (isFloatReturn(tIndex)) {
		double val = evaluateAssignmentReturnAsFloat(tIndex);
		return makeNumberAssignmentReturn((int)ceil(val));
	}
	else {
		int val = evaluateAssignmentReturnAsNumber(tIndex);
		return makeNumberAssignmentReturn(val);
	}
}

static AssignmentReturnValue evaluateIfElseArrayAssignment(AssignmentReturnValue tIndex) {
	char condText[100], yesText[100], noText[100];
	char comma1[20], comma2[20];

	sscanf(tIndex.mValue, "%s %s %s %s %s", condText, comma1, yesText, comma2, noText);
	assert(!strcmp(",", comma1));
	assert(!strcmp(",", comma2));
	assert(strcmp("", condText));
	assert(strcmp("", yesText));
	assert(strcmp("", noText));



	AssignmentReturnValue condRet = makeStringAssignmentReturn(condText);
	AssignmentReturnValue yesRet = makeStringAssignmentReturn(yesText);
	AssignmentReturnValue noRet = makeStringAssignmentReturn(noText);

	int cond = evaluateAssignmentReturnAsBool(condRet);

	if (cond) return yesRet;
	else return noRet;
}

static AssignmentReturnValue evaluateCondArrayAssignment(MugenAssignment* tCondVector, void* tCaller) {
	assert(tCondVector->mType == MUGEN_ASSIGNMENT_TYPE_VECTOR);
	MugenDependOnTwoAssignment* firstV = tCondVector->mData;
	assert(firstV->b->mType == MUGEN_ASSIGNMENT_TYPE_VECTOR);
	MugenDependOnTwoAssignment* secondV = firstV->b->mData;

	AssignmentReturnValue ret;
	int isTrue = evaluateMugenAssignment(firstV->a, tCaller);
	if (isTrue) {
		ret = evaluateAssignmentInternal(secondV->a, tCaller);
	}
	else {
		ret = evaluateAssignmentInternal(secondV->b, tCaller);
	}

	return ret;
}

static AssignmentReturnValue evaluateUserArrayAssignment(char* tVariableName, AssignmentReturnValue tIndex, void* tCaller) {
	AssignmentReturnValue ret;

	assert(string_map_contains(&gData.mArrays, tVariableName));
	MugenAssignmentArrayEntry* e = string_map_get(&gData.mArrays, tVariableName);

	e->mEvalFunc(ret.mValue, tCaller, tIndex.mValue);
	return ret;
}

static AssignmentReturnValue evaluateArrayAssignment(MugenAssignment* tAssignment, void* tCaller) {
	MugenDependOnTwoAssignment* arrays = tAssignment->mData;

	char test[100];
	assert(arrays->a->mType == MUGEN_ASSIGNMENT_TYPE_VARIABLE);
	MugenVariableAssignment* arrayName = arrays->a->mData;
	strcpy(test, arrayName->mName);
	turnStringLowercase(test);

	if (!strcmp("abs", test)) {
		AssignmentReturnValue b = evaluateAssignmentInternal(arrays->b, tCaller);
		return evaluateAbsArrayAssignment(b);
	}
	else if (!strcmp("exp", test)) {
		AssignmentReturnValue b = evaluateAssignmentInternal(arrays->b, tCaller);
		return evaluateExpArrayAssignment(b);
	}
	else if (!strcmp("ln", test)) {
		AssignmentReturnValue b = evaluateAssignmentInternal(arrays->b, tCaller);
		return evaluateNaturalLogArrayAssignment(b);
	}
	else if (!strcmp("cos", test)) {
		AssignmentReturnValue b = evaluateAssignmentInternal(arrays->b, tCaller);
		return evaluateCosineArrayAssignment(b);
	}
	else if (!strcmp("sin", test)) {
		AssignmentReturnValue b = evaluateAssignmentInternal(arrays->b, tCaller);
		return evaluateSineArrayAssignment(b);
	}
	else if (!strcmp("floor", test)) {
		AssignmentReturnValue b = evaluateAssignmentInternal(arrays->b, tCaller);
		return evaluateFloorArrayAssignment(b);
	}
	else if (!strcmp("ceil", test)) {
		AssignmentReturnValue b = evaluateAssignmentInternal(arrays->b, tCaller);
		return evaluateCeilArrayAssignment(b);
	}
	else if (!strcmp("ifelse", test) || !strcmp("sifelse", test)) {
		AssignmentReturnValue b = evaluateAssignmentInternal(arrays->b, tCaller);
		return evaluateIfElseArrayAssignment(b);
	}
	else if (!strcmp("cond", test)) {
		return evaluateCondArrayAssignment(arrays->b, tCaller);
	}

	if(!string_map_contains(&gData.mArrays, test)) {
		logError("Unknown array.");
		logErrorString(test);
		recoverFromError();
		return makeBooleanAssignmentReturn(0);
	}

	AssignmentReturnValue b = evaluateAssignmentInternal(arrays->b, tCaller);
	return evaluateUserArrayAssignment(test, b, tCaller);
}

static AssignmentReturnValue evaluateUnaryMinusAssignment(MugenAssignment* tAssignment, void* tCaller) {
	MugenDependOnOneAssignment* min = tAssignment->mData;

	AssignmentReturnValue a = evaluateAssignmentInternal(min->a, tCaller);
	if (isFloatReturn(a)) {
		return makeFloatAssignmentReturn(-evaluateAssignmentReturnAsFloat(a));
	}
	else {
		return makeNumberAssignmentReturn(-evaluateAssignmentReturnAsNumber(a));
	}
}

static AssignmentReturnValue evaluateNegationAssignment(MugenAssignment* tAssignment, void* tCaller) {
	MugenDependOnOneAssignment* neg = tAssignment->mData;

	AssignmentReturnValue a = evaluateAssignmentInternal(neg->a, tCaller);
	int val = evaluateAssignmentReturnAsBool(a);

	return makeBooleanAssignmentReturn(!val);
}

static AssignmentReturnValue evaluateAssignmentInternal(MugenAssignment* tAssignment, void* tCaller) {
	assert(tAssignment != NULL);
	assert(tAssignment->mData != NULL);


	AssignmentReturnValue ret;
	if (tAssignment->mType == MUGEN_ASSIGNMENT_TYPE_OR) {
		ret = evaluateOrAssignment(tAssignment, tCaller);
	}
	else if (tAssignment->mType == MUGEN_ASSIGNMENT_TYPE_AND) {
		return evaluateAndAssignment(tAssignment, tCaller);
	}
	else if (tAssignment->mType == MUGEN_ASSIGNMENT_TYPE_BITWISE_OR) {
		return evaluateBitwiseOrAssignment(tAssignment, tCaller);
	}
	else if (tAssignment->mType == MUGEN_ASSIGNMENT_TYPE_BITWISE_AND) {
		return evaluateBitwiseAndAssignment(tAssignment, tCaller);
	}
	else if (tAssignment->mType == MUGEN_ASSIGNMENT_TYPE_COMPARISON) {
		return evaluateComparisonAssignment(tAssignment, tCaller);
	}
	else if (tAssignment->mType == MUGEN_ASSIGNMENT_TYPE_FIXED_BOOLEAN) {
		return evaluateBooleanAssignment(tAssignment);
	}
	else if (tAssignment->mType == MUGEN_ASSIGNMENT_TYPE_VARIABLE) {
		return evaluateVariableAssignment(tAssignment, tCaller);
	}
	else if (tAssignment->mType == MUGEN_ASSIGNMENT_TYPE_NUMBER) {
		return evaluateNumberAssignment(tAssignment);
	}
	else if (tAssignment->mType == MUGEN_ASSIGNMENT_TYPE_VECTOR) {
		return evaluateVectorAssignment(tAssignment, tCaller);
	}
	else if (tAssignment->mType == MUGEN_ASSIGNMENT_TYPE_INEQUALITY) {
		return evaluateInequalityAssignment(tAssignment, tCaller);
	}
	else if (tAssignment->mType == MUGEN_ASSIGNMENT_TYPE_RANGE) {
		return evaluateRangeAssignment(tAssignment, tCaller);
	}
	else if (tAssignment->mType == MUGEN_ASSIGNMENT_TYPE_ARRAY) {
		return evaluateArrayAssignment(tAssignment, tCaller);
	}
	else if (tAssignment->mType == MUGEN_ASSIGNMENT_TYPE_NULL) {
		return evaluateBooleanAssignment(tAssignment);
	}
	else if (tAssignment->mType == MUGEN_ASSIGNMENT_TYPE_GREATER) {
		return evaluateGreaterAssignment(tAssignment, tCaller);
	}
	else if (tAssignment->mType == MUGEN_ASSIGNMENT_TYPE_GREATER_OR_EQUAL) {
		return evaluateGreaterOrEqualAssignment(tAssignment, tCaller);
	}
	else if (tAssignment->mType == MUGEN_ASSIGNMENT_TYPE_LESS) {
		return evaluateLessAssignment(tAssignment, tCaller);
	}
	else if (tAssignment->mType == MUGEN_ASSIGNMENT_TYPE_LESS_OR_EQUAL) {
		return evaluateLessOrEqualAssignment(tAssignment, tCaller);
	}
	else if (tAssignment->mType == MUGEN_ASSIGNMENT_TYPE_MODULO) {
		return evaluateModuloAssignment(tAssignment, tCaller);
	}
	else if (tAssignment->mType == MUGEN_ASSIGNMENT_TYPE_EXPONENTIATION) {
		return evaluateExponentiationAssignment(tAssignment, tCaller);
	}
	else if (tAssignment->mType == MUGEN_ASSIGNMENT_TYPE_MULTIPLICATION) {
		return evaluateMultiplicationAssignment(tAssignment, tCaller);
	}
	else if (tAssignment->mType == MUGEN_ASSIGNMENT_TYPE_DIVISION) {
		return evaluateDivisionAssignment(tAssignment, tCaller);
	}
	else if (tAssignment->mType == MUGEN_ASSIGNMENT_TYPE_ADDITION) {
		return evaluateAdditionAssignment(tAssignment, tCaller);
	}
	else if (tAssignment->mType == MUGEN_ASSIGNMENT_TYPE_SUBTRACTION) {
		return evaluateSubtractionAssignment(tAssignment, tCaller);
	}
	else if (tAssignment->mType == MUGEN_ASSIGNMENT_TYPE_OPERATOR_ARGUMENT) {
		return evaluateOperatorArgumentAssignment(tAssignment, tCaller);
	}
	else if (tAssignment->mType == MUGEN_ASSIGNMENT_TYPE_UNARY_MINUS) {
		return evaluateUnaryMinusAssignment(tAssignment, tCaller);
	}
	else if (tAssignment->mType == MUGEN_ASSIGNMENT_TYPE_NEGATION) {
		return evaluateNegationAssignment(tAssignment, tCaller);
	}
	else if (tAssignment->mType == MUGEN_ASSIGNMENT_TYPE_FLOAT) {
		return evaluateFloatAssignment(tAssignment);
	}
	else if (tAssignment->mType == MUGEN_ASSIGNMENT_TYPE_STRING) {
		return evaluateStringAssignment(tAssignment);
	}
	else {
		logError("Unidentified assignment type.");
		logErrorInteger(tAssignment->mType);
		recoverFromError();
		*ret.mValue = '\0';
	}

	return ret;
}

int evaluateMugenAssignment(MugenAssignment * tAssignment, void* tCaller)
{
	AssignmentReturnValue ret = evaluateAssignmentInternal(tAssignment, tCaller);
	return evaluateAssignmentReturnAsBool(ret);
}

double evaluateMugenAssignmentAndReturnAsFloat(MugenAssignment * tAssignment, void* tCaller)
{
	AssignmentReturnValue ret = evaluateAssignmentInternal(tAssignment, tCaller);
	return evaluateAssignmentReturnAsFloat(ret);
}

int evaluateMugenAssignmentAndReturnAsInteger(MugenAssignment * tAssignment, void* tCaller)
{
	AssignmentReturnValue ret = evaluateAssignmentInternal(tAssignment, tCaller);
	return evaluateAssignmentReturnAsNumber(ret);
}

char * evaluateMugenAssignmentAndReturnAsAllocatedString(MugenAssignment * tAssignment, void* tCaller)
{
	AssignmentReturnValue ret = evaluateAssignmentInternal(tAssignment, tCaller);
	char* str = allocMemory(strlen(ret.mValue) + 10);
	strcpy(str, ret.mValue);

	return str;
}

Vector3D evaluateMugenAssignmentAndReturnAsVector3D(MugenAssignment * tAssignment, void* tCaller)
{
	AssignmentReturnValue ret = evaluateAssignmentInternal(tAssignment, tCaller);

	double x, y, z;
	char tX[100], comma1[100], tY[100], comma2[100], tZ[100];

	int items = sscanf(ret.mValue, "%s %s %s %s %s", tX, comma1, tY, comma2, tZ);

	if (items >= 1) x = atof(tX);
	else x = 0;
	if (items >= 3) y = atof(tY);
	else y = 0;
	if (items >= 5) z = atof(tZ);
	else z = 0;

	return makePosition(x, y, z);
}


void resetMugenAssignmentContext() {
	gData.mVariables = new_string_map();
	gData.mArrays = new_string_map();
}

void addMugenAssignmentVariable(char* tVariable, void(*tEvalFunc)(char* tOutput, void* tCaller)) {
	MugenAssignmentVariableEntry* e = allocMemory(sizeof(MugenAssignmentVariableEntry));
	e->mEvalFunc = tEvalFunc;
	char varText[100];
	strcpy(varText, tVariable);
	turnStringLowercase(varText);
	string_map_push_owned(&gData.mVariables, varText, e);
}

void addMugenAssignmentArray(char * tVariable, void(*tEvalFunc)(char *tOutput, void *tCaller, char *tArrayIndex))
{
	MugenAssignmentArrayEntry* e = allocMemory(sizeof(MugenAssignmentArrayEntry));
	e->mEvalFunc = tEvalFunc;
	char varText[100];
	strcpy(varText, tVariable);
	turnStringLowercase(varText);
	string_map_push_owned(&gData.mArrays, varText, e);
}


int getMugenAssignmentAsIntegerValueOrDefaultWhenEmpty(MugenAssignment* tAssignment, void* tCaller, int tDefault) {
	char* flag = evaluateMugenAssignmentAndReturnAsAllocatedString(tAssignment, tCaller);

	int ret;
	if (!strcmp("", flag)) ret = tDefault;
	else ret = atoi(flag);

	freeMemory(flag);
	return ret;
}

double getMugenAssignmentAsFloatValueOrDefaultWhenEmpty(MugenAssignment* tAssignment, void* tCaller, double tDefault) {
	char* flag = evaluateMugenAssignmentAndReturnAsAllocatedString(tAssignment, tCaller);

	double ret;
	if (!strcmp("", flag)) ret = tDefault;
	else ret = atof(flag);

	freeMemory(flag);
	return ret;
}

Vector3D getMugenAssignmentAsVector3DValueOrDefaultWhenEmpty(MugenAssignment* tAssignment, void* tCaller, Vector3D tDefault) {
	char* flag = evaluateMugenAssignmentAndReturnAsAllocatedString(tAssignment, tCaller);

	Vector3D ret;
	if (!strcmp("", flag)) ret = tDefault;
	else ret = evaluateMugenAssignmentAndReturnAsVector3D(tAssignment, tCaller);

	freeMemory(flag);
	return ret;
}