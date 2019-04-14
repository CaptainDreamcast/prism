#include "prism/mugenassignment.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include "prism/memoryhandler.h"
#include "prism/log.h"
#include "prism/system.h"
#include "prism/math.h"

static MugenAssignment* makeMugenAssignment(MugenAssignmentType tType, void* tData) {
	MugenAssignment* e = (MugenAssignment*)allocMemory(sizeof(MugenAssignment));
	e->mType = tType;
	e->mData = tData;
	return e;
}

MugenAssignment * makeTrueMugenAssignment()
{
	MugenFixedBooleanAssignment* data = (MugenFixedBooleanAssignment*)allocMemory(sizeof(MugenFixedBooleanAssignment));
	data->mValue = 1;
	return makeMugenAssignment(MUGEN_ASSIGNMENT_TYPE_FIXED_BOOLEAN, data);
}

void destroyFalseMugenAssignment(MugenAssignment* tAssignment) {
	freeMemory(tAssignment->mData);
	freeMemory(tAssignment);
}

void destroyMugenAssignment(MugenAssignment * tAssignment)
{
	// TODO
	freeMemory(tAssignment->mData);
	freeMemory(tAssignment);
}

MugenAssignment * makeFalseMugenAssignment()
{
	MugenFixedBooleanAssignment* data = (MugenFixedBooleanAssignment*)allocMemory(sizeof(MugenFixedBooleanAssignment));
	data->mValue = 0;
	return makeMugenAssignment(MUGEN_ASSIGNMENT_TYPE_FIXED_BOOLEAN, data);
}

MugenAssignment * makeMugenOneElementAssignment(MugenAssignmentType tType, MugenAssignment * a) 
{

	MugenDependOnOneAssignment* data = (MugenDependOnOneAssignment*)allocMemory(sizeof(MugenDependOnOneAssignment));
	data->a = a;
	return makeMugenAssignment(tType, data);
}


MugenAssignment * makeMugenTwoElementAssignment(MugenAssignmentType tType, MugenAssignment * a, MugenAssignment * b)
{
	MugenDependOnTwoAssignment* data = (MugenDependOnTwoAssignment*)allocMemory(sizeof(MugenDependOnTwoAssignment));
	data->a = a;
	data->b = b;
	return makeMugenAssignment(tType, data);
}



MugenAssignment * makeNumberMugenAssignment(int tVal)
{

	MugenNumberAssignment* number = (MugenNumberAssignment*)allocMemory(sizeof(MugenNumberAssignment));
	number->mValue = tVal;

	return makeMugenAssignment(MUGEN_ASSIGNMENT_TYPE_NUMBER, number);
}

MugenAssignment * makeFloatMugenAssignment(double tVal)
{
	MugenFloatAssignment* f = (MugenFloatAssignment*)allocMemory(sizeof(MugenFloatAssignment));
	f->mValue = tVal;

	return makeMugenAssignment(MUGEN_ASSIGNMENT_TYPE_FLOAT, f);
}

MugenAssignment * makeStringMugenAssignment(char * tVal)
{
	MugenStringAssignment* s = (MugenStringAssignment*)allocMemory(sizeof(MugenStringAssignment));
	s->mValue = (char*)allocMemory(strlen(tVal) + 10);
	strcpy(s->mValue, tVal);

	return makeMugenAssignment(MUGEN_ASSIGNMENT_TYPE_STRING, s);
}

MugenAssignment * make2DVectorMugenAssignment(Vector3D tVal)
{
	MugenDependOnTwoAssignment* data = (MugenDependOnTwoAssignment*)allocMemory(sizeof(MugenDependOnTwoAssignment));
	data->a = makeFloatMugenAssignment(tVal.x);
	data->b = makeFloatMugenAssignment(tVal.y);
	return makeMugenAssignment(MUGEN_ASSIGNMENT_TYPE_VECTOR, data);
}

MugenAssignment * makeAndMugenAssignment(MugenAssignment * a, MugenAssignment * b)
{
	return makeMugenTwoElementAssignment(MUGEN_ASSIGNMENT_TYPE_AND, a, b);
}

MugenAssignment * makeOrMugenAssignment(MugenAssignment * a, MugenAssignment * b)
{
	return makeMugenTwoElementAssignment(MUGEN_ASSIGNMENT_TYPE_OR, a, b);
}

static int isOnHighestLevelWithStartPosition(char* tText, char* tPattern, int* tOptionalPosition, int tStart) {
	int n = strlen(tText);
	int m = strlen(tPattern);

	int depth1 = 0;
	int depth2 = 0;
	int depth3 = 0;
	int i;
	for (i = tStart; i < n - m + 1; i++) {
		if (tText[i] == '(') depth1++;
		if (tText[i] == ')') depth1--;
		if (tText[i] == '[') depth2++;
		if (tText[i] == ']') depth2--;
		if (tText[i] == '"') depth3 ^= 1;

		if (depth1 || depth2 || depth3) continue;

		int isSame = 1;
		int j;
		for (j = 0; j < m; j++) {
			if (tText[i + j] != tPattern[j]) {
				isSame = 0;
				break;
			}
		}

		if (isSame) {
			if (tOptionalPosition) *tOptionalPosition = i;
			return 1;
		}
	}

	return 0;
}

static int isOnHighestLevel(char* tText, char* tPattern, int* tOptionalPosition) {
	return isOnHighestLevelWithStartPosition(tText, tPattern, tOptionalPosition, 0);
}

static MugenAssignment* parseOneElementMugenAssignmentFromString(char* tText, MugenAssignmentType tType) {
	
	MugenAssignment* a = parseMugenAssignmentFromString(tText);

	return makeMugenOneElementAssignment(tType, a);
}

static MugenAssignment* parseTwoElementMugenAssignmentFromStringWithFixedPosition(char* tText, MugenAssignmentType tType, char* tPattern, int tPosition) {
	int n = strlen(tText);
	int m = strlen(tPattern);

	assert(tPosition != -1);
	assert(tPosition > 0);
	assert(tPosition < n - m);

	int start = tPosition;
	int end = tPosition + m;

	char text1[MUGEN_DEF_STRING_LENGTH];
	char text2[MUGEN_DEF_STRING_LENGTH];

	strcpy(text1, tText);
	text1[start] = '\0';

	strcpy(text2, &tText[end]);

	MugenAssignment* a = parseMugenAssignmentFromString(text1);
	MugenAssignment* b = parseMugenAssignmentFromString(text2);

	return makeMugenTwoElementAssignment(tType, a, b);
}

static MugenAssignment* parseTwoElementMugenAssignmentFromString(char* tText, MugenAssignmentType tType, char* tPattern) {
	int pos = -1;
	isOnHighestLevel(tText, tPattern, &pos);
	return parseTwoElementMugenAssignmentFromStringWithFixedPosition(tText, tType, tPattern, pos);
}

static int isEmpty(char* tChar) {
	return !strcmp("", tChar);
}

static int isEmptyCharacter(char tChar) {
	return tChar == ' ';
}

static MugenAssignment* parseMugenNullFromString() {
	MugenFixedBooleanAssignment* data = (MugenFixedBooleanAssignment*)allocMemory(sizeof(MugenFixedBooleanAssignment));
	data->mValue = 0;
	return makeMugenAssignment(MUGEN_ASSIGNMENT_TYPE_NULL, data);
}



static int isInBraces(char* tText) {
	int n = strlen(tText);
	if (tText[0] != '(' || tText[n - 1] != ')') return 0;

	int depth = 0;
	int i;
	for (i = 0; i < n - 1; i++) {
		if (tText[i] == '(') depth++;
		if (tText[i] == ')') depth--;

		if (!depth) return 0;
	}

	return 1;
}

static MugenAssignment* parseMugenAssignmentStringInBraces(char* tText) {
	int n = strlen(tText);
	tText[n - 1] = '\0';
	tText++;

	return parseMugenAssignmentFromString(tText);
}

static int isNegation(char* tText) {
	return (tText[0] == '!');
}

static MugenAssignment* parseMugenNegationFromString(char* tText) {
	tText++;
	return parseOneElementMugenAssignmentFromString(tText, MUGEN_ASSIGNMENT_TYPE_NEGATION);
}

static int isUnaryMinus(char* tText) {
	return (tText[0] == '-');
}

static MugenAssignment* parseMugenUnaryMinusFromString(char* tText) {
	tText++;
	return parseOneElementMugenAssignmentFromString(tText, MUGEN_ASSIGNMENT_TYPE_UNARY_MINUS);
}

static int isRange(char* tText) {
	int n = strlen(tText);
	if ((tText[0] != '[' && tText[0] != '(') || (tText[n - 1] != ']' && tText[n - 1] != ')')) return 0;

	int depth = 0;
	int i;
	for (i = 0; i < n - 1; i++) {
		if (tText[i] == '(' || tText[i] == '[') depth++;
		if (tText[i] == ')' || tText[i] == ']') depth--;

		if (!depth) return 0;
	}

	return 1;
}

static MugenAssignment* parseMugenRangeFromString(char* tText) {
	MugenRangeAssignment* e = (MugenRangeAssignment*)allocMemory(sizeof(MugenRangeAssignment));
	int n = strlen(tText);
	e->mExcludeLeft = tText[0] == '(';
	e->mExcludeRight = tText[n - 1] == ')';
	
	tText[n - 1] = '\0';
	tText++;

	e->a = parseMugenAssignmentFromString(tText);
	return makeMugenAssignment(MUGEN_ASSIGNMENT_TYPE_RANGE, e);
}


static int isInequality(char* tText) {
	return isOnHighestLevel(tText, "!=", NULL);
}

static MugenAssignment* parseMugenInequalityFromString(char* tText) {
	return parseTwoElementMugenAssignmentFromString(tText, MUGEN_ASSIGNMENT_TYPE_INEQUALITY, "!=");
}

static int isComparison(char* tText) {
	int position;

	// TODO: check for multiple
	if(!isOnHighestLevel(tText, "=", &position)) return 0;
	if (position == 0) return 0;
	if (tText[position - 1] == '!') return 0;
	if (tText[position - 1] == '<') return 0;
	if (tText[position - 1] == '>') return 0;
	if (tText[position - 1] == ':') return 0;

	return 1;
}

static MugenAssignment* parseMugenComparisonFromString(char* tText) {
	return parseTwoElementMugenAssignmentFromString(tText, MUGEN_ASSIGNMENT_TYPE_COMPARISON, "=");
}

static int isAnd(char* tText) {
	return isOnHighestLevel(tText, "&&", NULL);
}

static MugenAssignment* parseMugenAndFromString(char* tText) {
	return parseTwoElementMugenAssignmentFromString(tText, MUGEN_ASSIGNMENT_TYPE_AND, "&&");
}

static int isOr(char* tText) {
	return isOnHighestLevel(tText, "||", NULL);
}

static MugenAssignment* parseMugenOrFromString(char* tText) {
	return parseTwoElementMugenAssignmentFromString(tText, MUGEN_ASSIGNMENT_TYPE_OR, "||");
}

static int isBitwiseAnd(char* tText) {
	return isOnHighestLevel(tText, "&", NULL);
}

static MugenAssignment* parseMugenBitwiseAndFromString(char* tText) {
	return parseTwoElementMugenAssignmentFromString(tText, MUGEN_ASSIGNMENT_TYPE_BITWISE_AND, "&");
}

static int isBitwiseOr(char* tText) {
	return isOnHighestLevel(tText, "|", NULL);
}

static MugenAssignment* parseMugenBitwiseOrFromString(char* tText) {
	return parseTwoElementMugenAssignmentFromString(tText, MUGEN_ASSIGNMENT_TYPE_BITWISE_OR, "|");
}

static int isLessOrEqualThan(char* tText) {
	int position;
	if (!isOnHighestLevel(tText, "<=", &position)) return 0;
	if (position == 0) return 0;

	return 1;
}

static MugenAssignment* parseMugenLessOrEqualFromString(char* tText) {
	return parseTwoElementMugenAssignmentFromString(tText, MUGEN_ASSIGNMENT_TYPE_LESS_OR_EQUAL, "<=");
}

static int isLessThan(char* tText) {
	int position;
	if (!isOnHighestLevel(tText, "<", &position)) return 0;
	if (position == 0) return 0;

	return 1;
}

static MugenAssignment* parseMugenLessFromString(char* tText) {
	return parseTwoElementMugenAssignmentFromString(tText, MUGEN_ASSIGNMENT_TYPE_LESS, "<");
}

static int isGreaterOrEqualThan(char* tText) {
	int position;
	if (!isOnHighestLevel(tText, ">=", &position)) return 0;
	if (position == 0) return 0;

	return 1;
}

static MugenAssignment* parseMugenGreaterOrEqualFromString(char* tText) {
	return parseTwoElementMugenAssignmentFromString(tText, MUGEN_ASSIGNMENT_TYPE_GREATER_OR_EQUAL, ">=");
}

static int isGreaterThan(char* tText) {
	int position;
	if(!isOnHighestLevel(tText, ">", &position)) return 0;
	if (position == 0) return 0;

	return 1;
}

static MugenAssignment* parseMugenGreaterFromString(char* tText) {
	return parseTwoElementMugenAssignmentFromString(tText, MUGEN_ASSIGNMENT_TYPE_GREATER, ">");
}

static int isExponentiation(char* tText) {
	return isOnHighestLevel(tText, "**", NULL);
}

static MugenAssignment* parseMugenExponentiationFromString(char* tText) {
	return parseTwoElementMugenAssignmentFromString(tText, MUGEN_ASSIGNMENT_TYPE_EXPONENTIATION, "**");
}

static int isAddition(char* tText) {
	return isOnHighestLevel(tText, "+", NULL);
}

static MugenAssignment* parseMugenAdditionFromString(char* tText) {
	return parseTwoElementMugenAssignmentFromString(tText, MUGEN_ASSIGNMENT_TYPE_ADDITION, "+");
}

static int isOperatorCharacter(char tChar) {
	return tChar == '-' || tChar == '+' || tChar == '|' || tChar == '&' || tChar == '*' || tChar == '/';
}

static int isBinaryOperator(char* tText, int tPosition) {
	int p = tPosition - 1;
	int poss = 0;
	while (p >= 0) {
		if (isEmptyCharacter(tText[p])) p--;
		else if (isOperatorCharacter(tText[p])) return 0;
		else {
			poss = 1;
			break;
		}
	}

	if (!poss) return 0;

	int n = strlen(tText);
	p = tPosition + 1;
	poss = 0;
	while (p < n) {
		if (isEmptyCharacter(tText[p])) p++;
		else if (isOperatorCharacter(tText[p])) return 0;
		else {
			poss = 1;
			break;
		}
	}

	return poss;
}

static int isSubtraction(char* tText) {
	int position;
	if(!isOnHighestLevel(tText, "-", &position)) return 0;
	return isBinaryOperator(tText, position);
}

static MugenAssignment* parseMugenSubtractionFromString(char* tText) {
	return parseTwoElementMugenAssignmentFromString(tText, MUGEN_ASSIGNMENT_TYPE_SUBTRACTION, "-");
}

static int isMultiplication(char* tText) {
	int position = 0;
	int n = strlen(tText);

	// TODO: check for multiple
	if (!isOnHighestLevel(tText, "*", &position)) return 0;
	if (position == 0) return 0;
	if (position < n-1 && tText[position + 1] == '*') return 0;

	return 1;
}

static MugenAssignment* parseMugenMultiplicationFromString(char* tText) {
	return parseTwoElementMugenAssignmentFromString(tText, MUGEN_ASSIGNMENT_TYPE_MULTIPLICATION, "*");
}

static int isDivision(char* tText) {
	return isOnHighestLevel(tText, "/", NULL);
}

static MugenAssignment* parseMugenDivisionFromString(char* tText) {
	return parseTwoElementMugenAssignmentFromString(tText, MUGEN_ASSIGNMENT_TYPE_DIVISION, "/");
}

static int isModulo(char* tText) {
	return isOnHighestLevel(tText, "%", NULL);
}

static MugenAssignment* parseMugenModuloFromString(char* tText) {
	return parseTwoElementMugenAssignmentFromString(tText, MUGEN_ASSIGNMENT_TYPE_MODULO, "%");
}

static int isNumericalConstant(char* tText) {
	if (*tText == '-') tText++;

	int n = strlen(tText);
	if (n == 0) return 0;

	int mPointAmount = 0;
	int i;
	for (i = 0; i < n; i++) {
		if (tText[i] == '.') mPointAmount++;
		else if (tText[i] >= '0' && tText[i] <= '9') continue;
		else return 0;
	}

	return mPointAmount <= 0;
}

static MugenAssignment* parseNumericalConstantFromString(char* tText) {
	int val = atoi(tText);

	return makeNumberMugenAssignment(val);
}

static int isFloatConstant(char* tText) {
	if (*tText == '-') tText++;

	int n = strlen(tText);
	if (n == 0) return 0;

	int mPointAmount = 0;
	int i;
	for (i = 0; i < n; i++) {
		if (tText[i] == '.') mPointAmount++;
		else if (tText[i] >= '0' && tText[i] <= '9') continue;
		else return 0;
	}

	return mPointAmount <= 1;
}

static MugenAssignment* parseFloatConstantFromString(char* tText) {
	double f = atof(tText);

	return makeFloatMugenAssignment(f);
}

static int isStringConstant(char* tText) {
	int n = strlen(tText);
	if (n == 0) return 0;
	return tText[0] == '"' && tText[n - 1] == '"';
}

static MugenAssignment* parseStringConstantFromString(char* tText) {
	MugenStringAssignment* s = (MugenStringAssignment*)allocMemory(sizeof(MugenStringAssignment));
	s->mValue = (char*)allocMemory(strlen(tText + 1) + 10);
	strcpy(s->mValue, tText+1);
	s->mValue[strlen(s->mValue) - 1] = '\0';

	return makeMugenAssignment(MUGEN_ASSIGNMENT_TYPE_STRING, s);
}

int doMugenAssignmentStringsBeginsWithPattern(char* tPattern, char* tText) {
	int n = strlen(tPattern);
	int m = strlen(tText);
	if (m < n) return 0;

	int i;
	for (i = 0; i < n; i++) {
		if (tPattern[i] != tText[i]) return 0;
	}

	return 1;
}

static int isVariable(char* tText) {
	(void)tText; // TODO: check
	return 1;
}

static MugenAssignment* parseMugenVariableFromString(char* tText) {
	MugenVariableAssignment* data = (MugenVariableAssignment*)allocMemory(sizeof(MugenVariableAssignment));
	strcpy(data->mName, tText);

	return makeMugenAssignment(MUGEN_ASSIGNMENT_TYPE_VARIABLE, data);
}

static int isArray(char* tText) {

	int n = strlen(tText);
	char* open = strchr(tText, '(');
	char* close = strrchr(tText, ')');

	return open != NULL && close != NULL && close > open && open != tText && close == (tText + n - 1);
}

static MugenAssignment* parseArrayFromString(char* tText) {
	int posOpen = -1, posClose = -1;
	posOpen = strchr(tText, '(') - tText;
	posClose = strrchr(tText, ')') - tText;
	assert(posOpen >= 0);
	assert(posClose >= 0);

	char text1[MUGEN_DEF_STRING_LENGTH];
	char text2[MUGEN_DEF_STRING_LENGTH];

	strcpy(text1, tText);
	text1[posOpen] = '\0';

	strcpy(text2, &tText[posOpen+1]);
	char* text2End = strrchr(text2, ')');
	*text2End = '\0';

	MugenAssignment* a = parseMugenAssignmentFromString(text1);
	MugenAssignment* b = parseMugenAssignmentFromString(text2);

	return makeMugenTwoElementAssignment(MUGEN_ASSIGNMENT_TYPE_ARRAY, a, b);
}

static int isVectorAssignment(char* tText) {
	if (!doMugenAssignmentStringsBeginsWithPattern("AnimElem", tText)) return 0;

	// TODO: properly
	return 1;
}

static int isVectorTarget(char* tText) {

	// TODO: make general
	if (doMugenAssignmentStringsBeginsWithPattern("target", tText)) return 1;
	if (doMugenAssignmentStringsBeginsWithPattern("p1", tText)) return 1;
	if (doMugenAssignmentStringsBeginsWithPattern("p2", tText)) return 1;
	if (doMugenAssignmentStringsBeginsWithPattern("helper", tText)) return 1;
	if (doMugenAssignmentStringsBeginsWithPattern("enemy", tText)) return 1;
	if (doMugenAssignmentStringsBeginsWithPattern("enemynear", tText)) return 1;
	if (doMugenAssignmentStringsBeginsWithPattern("root", tText)) return 1;
	if (doMugenAssignmentStringsBeginsWithPattern("playerid", tText)) return 1;
	if (doMugenAssignmentStringsBeginsWithPattern("parent", tText)) return 1;

	// TODO: properly
	return 0;
}

static int isCommaContextFree(char* tText, int tPosition) {
	assert(tText[tPosition] == ',');
	tPosition--;
	while (tPosition >= 0 && isEmptyCharacter(tText[tPosition])) tPosition--;
	assert(tPosition >= 0);
	int end = tPosition+1;

	int depth1 = 0;
	int depth2 = 0;
	while (tPosition >= 0) {
		if (tText[tPosition] == ')') depth1++;
		if (tText[tPosition] == '(') {
			if (!depth1) break;
			else depth1--;
		}
		if (tText[tPosition] == ']') depth2++;
		if (tText[tPosition] == '[') {
			if (!depth2) break;
			else depth2--;
		}

		if (!depth1 && !depth2 && (tText[tPosition] == ',' || isEmptyCharacter(tText[tPosition]) || isOperatorCharacter(tText[tPosition]))) break;
		tPosition--;
	}
	assert(tPosition >= -1);
	int start = tPosition + 1;

	char prevWord[200];
	strcpy(prevWord, &tText[start]);
	int length = end - start;
	prevWord[length] = '\0';

	return !isVectorTarget(prevWord);
}

static int hasContextFreeComma(char* tText, int* tPosition) {
	int isRunning = 1;
	int position = 0;
	while (isRunning) {
		if (!isOnHighestLevelWithStartPosition(tText, ",", &position, position)) return 0;
		if (isCommaContextFree(tText, position)) {
			if (tPosition) *tPosition = position;
			return 1;
		}

		position++;
	}

	return 0;
}

static int isVector(char* tText) {
	return hasContextFreeComma(tText, NULL) && !isVectorAssignment(tText);
}

static MugenAssignment* parseMugenContextFreeVectorFromString(char* tText) {
	int position;
	hasContextFreeComma(tText, &position);

	// TODO: handle when second element is gone
	return parseTwoElementMugenAssignmentFromStringWithFixedPosition(tText, MUGEN_ASSIGNMENT_TYPE_VECTOR, ",", position);
}

static int isVectorInsideAssignmentOrTargetAccess(char* tText) {
	return isOnHighestLevel(tText, ",", NULL);
}

static MugenAssignment* parseMugenVectorFromString(char* tText) {
	return parseTwoElementMugenAssignmentFromString(tText, MUGEN_ASSIGNMENT_TYPE_VECTOR, ",");
}

static int isOperatorAndReturnType(char* tText, char* tDst) {
	int position;
	int isThere;
	int isDifferent;

	isThere = isOnHighestLevel(tText, ">=", &position);
	isDifferent = strcmp(">=", tText);
	if (isThere && isDifferent && position == 0) {
		strcpy(tDst, ">=");
		return 1;
	}

	isThere = isOnHighestLevel(tText, "<=", &position);
	isDifferent = strcmp("<=", tText);
	if (isThere && isDifferent && position == 0) {
		strcpy(tDst, "<=");
		return 1;
	}

	isThere = isOnHighestLevel(tText, "=", &position);
	isDifferent = strcmp("=", tText);
	if (isThere && isDifferent && position == 0) {
		strcpy(tDst, "=");
		return 1;
	}

	return 0;
}

static int isOperatorArgument(char* tText) {
	char dst[10];

	int ret = isOperatorAndReturnType(tText, dst);
	return ret;
}

static MugenAssignment* parseMugenOperatorArgumentFromString(char* tText) {
	char dst[10];
	char text[200];
	isOperatorAndReturnType(tText, dst);

	sprintf(text, "%s $$ %s", dst, tText + strlen(dst));
	return parseTwoElementMugenAssignmentFromString(text, MUGEN_ASSIGNMENT_TYPE_OPERATOR_ARGUMENT, "$$");
}

static void sanitizeTextFront(char** tText) {
	int n = strlen(*tText);
	int i;
	for (i = 0; i < n; i++) {
		if (**tText != ' ') {
			return;
		}

		(*tText)++;
	}
}

static void sanitizeTextBack(char* tText) {
	int n = strlen(tText);

	int i;
	for (i = n - 1; i >= 0; i--) {
		if (tText[i] == ' ') tText[i] = '\0';
		else return;
	}
}

static void sanitizeText(char** tText) {
	sanitizeTextFront(tText);
	sanitizeTextBack(*tText);
}

MugenAssignment * parseMugenAssignmentFromString(char * tText)
{
	sanitizeText(&tText);

	if (isEmpty(tText)) {
		return parseMugenNullFromString();
	}
	else if (isVector(tText)) {
		return parseMugenContextFreeVectorFromString(tText);
	}
	else if (isOperatorArgument(tText)) {
		return parseMugenOperatorArgumentFromString(tText);
	}
	else if (isOr(tText)) {
		return parseMugenOrFromString(tText);
	}
	else if (isAnd(tText)) {
		return parseMugenAndFromString(tText);
	}
	else if (isBitwiseOr(tText)) {
		return parseMugenBitwiseOrFromString(tText);
	}
	else if (isBitwiseAnd(tText)) {
		return parseMugenBitwiseAndFromString(tText);
	}
	else if (isInequality(tText)) {
		return parseMugenInequalityFromString(tText);
	}
	else if (isComparison(tText)) {
		return parseMugenComparisonFromString(tText);
	}
	else if (isGreaterOrEqualThan(tText)) {
		return parseMugenGreaterOrEqualFromString(tText);
	}
	else if (isLessOrEqualThan(tText)) {
		return parseMugenLessOrEqualFromString(tText);
	}
	else if (isLessThan(tText)) {
		return parseMugenLessFromString(tText);
	}
	else if (isGreaterThan(tText)) {
		return parseMugenGreaterFromString(tText);
	}
	else if (isAddition(tText)) {
		return parseMugenAdditionFromString(tText);
	}
	else if (isSubtraction(tText)) {
		return parseMugenSubtractionFromString(tText);
	}
	else if (isModulo(tText)) {
		return parseMugenModuloFromString(tText);
	}
	else if (isMultiplication(tText)) {
		return parseMugenMultiplicationFromString(tText);
	}
	else if (isDivision(tText)) {
		return parseMugenDivisionFromString(tText);
	}
	else if (isExponentiation(tText)) {
		return parseMugenExponentiationFromString(tText);
	}
	else if (isNegation(tText)) {
		return parseMugenNegationFromString(tText);
	}
	else if (isUnaryMinus(tText)) {
		return parseMugenUnaryMinusFromString(tText);
	}
	else if (isInBraces(tText)) {
		return parseMugenAssignmentStringInBraces(tText);
	}
	else if (isRange(tText)) {
		return parseMugenRangeFromString(tText);
	}
	else if (isNumericalConstant(tText)) {
		return parseNumericalConstantFromString(tText);
	}
	else if (isFloatConstant(tText)) {
		return parseFloatConstantFromString(tText);
	}
	else if (isStringConstant(tText)) {
		return parseStringConstantFromString(tText);
	}
	else if (isVectorInsideAssignmentOrTargetAccess(tText)) {
		return parseMugenVectorFromString(tText);
	}
	else if (isArray(tText)) {
		return parseArrayFromString(tText);
	}
	else if (isVariable(tText)) {
		return parseMugenVariableFromString(tText);
	}
	else {
		logError("Unable to determine Mugen assignment.");
		logErrorString(tText);
		recoverFromError();
	}
	
	return NULL;
}


int fetchMugenAssignmentFromGroupAndReturnWhetherItExists(char* tName, MugenDefScriptGroup* tGroup, MugenAssignment** tOutput) {
	if (!stl_string_map_contains_array(tGroup->mElements, tName)) return 0;

	MugenDefScriptGroupElement* e = &tGroup->mElements[tName];
	char* text = getAllocatedMugenDefStringVariableAsElement(e);
	*tOutput = parseMugenAssignmentFromString(text);
	freeMemory(text);

	return 1;
}

void fetchMugenAssignmentFromGroupAndReturnWhetherItExistsDefaultString(char* tName, MugenDefScriptGroup* tGroup, MugenAssignment** tDst, char* tDefault) {
	if (!fetchMugenAssignmentFromGroupAndReturnWhetherItExists(tName, tGroup, tDst)) {
		*tDst = makeStringMugenAssignment(tDefault);
	}
}