#include "prism/mugendefwriter.h"

#include <assert.h>
#include <sstream>
#include <string>

#include "prism/datastructures.h"
#include "prism/log.h"

ModifiableMugenDefScript openModifiableMugenDefScript(const std::string& tPath) {
	if (!isFile(tPath)) {
		auto b = copyStringToBuffer("\n");
		bufferToFile(tPath.c_str(), b);
		freeBuffer(b);
	}

	ModifiableMugenDefScript ret;
	ret.mOwnedBuffer = makeBufferOwnedIfNecessary(fileToBuffer(tPath.c_str()));
	return ret;
}

void closeModifiableMugenDefScript(ModifiableMugenDefScript* tScript)
{
	freeBuffer(tScript->mOwnedBuffer);
}

void saveModifiableMugenDefScript(ModifiableMugenDefScript* tScript, const std::string& tPath) {
	bufferToFile(tPath.c_str(), tScript->mOwnedBuffer);
}

void clearModifiableMugenDefScript(ModifiableMugenDefScript* tScript)
{
	freeBuffer(tScript->mOwnedBuffer);
	tScript->mOwnedBuffer = copyStringToBuffer("\n");
}

static BufferPointer getStartOfModifiableMugenDefScript(ModifiableMugenDefScript* tScript) {
	auto p = getBufferPointer(tScript->mOwnedBuffer);
	return p;
}

static void replaceStringInModifiableMugenDefScript(ModifiableMugenDefScript* tScript, BufferPointer writePos, const std::string& tValue, size_t replacedSize) {
	const auto preWriteSize = size_t(writePos - ((char*)tScript->mOwnedBuffer.mData));
	const auto configPart1 = std::string((char*)tScript->mOwnedBuffer.mData, preWriteSize);
	const auto configPart2 = std::string(writePos + replacedSize, tScript->mOwnedBuffer.mLength - preWriteSize - replacedSize);
	const auto finalString = configPart1 + tValue + configPart2;
	freeBuffer(tScript->mOwnedBuffer);
	tScript->mOwnedBuffer = copyStringToBuffer(finalString);
}

static void insertStringIntoModifiableMugenDefScript(ModifiableMugenDefScript* tScript, BufferPointer writePos, const std::string& tValue) {
	replaceStringInModifiableMugenDefScript(tScript, writePos, tValue, 0);
}

static void addModifiableMugenDefScriptGroup(ModifiableMugenDefScript* tScript, const char* tGroupName) {
	auto writePos = getStartOfModifiableMugenDefScript(tScript);
	insertStringIntoModifiableMugenDefScript(tScript, writePos, std::string("[") + tGroupName + "]\n");
}

static BufferPointer findExistingVariablePositionOrNullIfNonExistant(ModifiableMugenDefScript* tScript, const char* tGroupName, const char* tVariableName) {
	auto p = getBufferPointer(tScript->mOwnedBuffer);

	int found = 0;
	while (true) {
		const auto line = readLineOrEOFFromTextStreamBufferPointer(&p, tScript->mOwnedBuffer);
		if (isBufferPointerOver(p, tScript->mOwnedBuffer)) break;
		if (line.size() > 0 && line[0] != '[') continue;
		std::string groupName;
		for (size_t i = 1; i < line.size() && line[i] != ']'; i++) {
			groupName.push_back(line[i]);
		}
		if (stringEqualCaseIndependent(groupName.c_str(), tGroupName)) {
			found = 1;
			break;
		}
	}

	if (!found) return NULL;

	std::vector<std::pair<std::string, BufferPointer>> relevantLines;
	while (true) {
		auto linePosition = p;
		const auto line = readLineOrEOFFromTextStreamBufferPointer(&p, tScript->mOwnedBuffer);
		if (isBufferPointerOver(p, tScript->mOwnedBuffer)) break;
		if (line.size() > 0 && line[0] == '[') break;
		relevantLines.push_back(std::make_pair(line, linePosition));
	}

	for (size_t i = 0; i < relevantLines.size(); i++) {
		std::istringstream ss(relevantLines[i].first);
		std::string firstWord;
		ss >> firstWord;
		if (stringEqualCaseIndependent(firstWord.c_str(), tVariableName)) {
			return relevantLines[i].second;
		}
	}

	return NULL;
}

static BufferPointer findStartOfGroup(ModifiableMugenDefScript* tScript, const char* tGroupName) {
	auto p = getBufferPointer(tScript->mOwnedBuffer);

	while (true) {
		const auto line = readLineOrEOFFromTextStreamBufferPointer(&p, tScript->mOwnedBuffer);
		if (isBufferPointerOver(p, tScript->mOwnedBuffer)) break;
		if (line.size() > 0 && line[0] != '[') continue;
		std::string groupName;
		for (size_t i = 1; i < line.size() && line[i] != ']'; i++) {
			groupName.push_back(line[i]);
		}
		if (stringEqualCaseIndependent(groupName.c_str(), tGroupName)) {
			return p;
		}
	}

	return NULL;
}

static int hasModifiableMugenDefScriptGroup(ModifiableMugenDefScript* tScript, const char* tGroupName) {
	auto p = getBufferPointer(tScript->mOwnedBuffer);

	while (true) {
		const auto line = readLineOrEOFFromTextStreamBufferPointer(&p, tScript->mOwnedBuffer);
		if (isBufferPointerOver(p, tScript->mOwnedBuffer)) break;
		if (line.size() > 0 && line[0] != '[') continue;
		std::string groupName;
		for (size_t i = 1; i < line.size() && line[i] != ']'; i++) {
			groupName.push_back(line[i]);
		}
		if (stringEqualCaseIndependent(groupName.c_str(), tGroupName)) {
			return 1;
		}
	}

	return 0;
}

static int isEmptyCharacter(char tChar) {
	return tChar == ' ';
}

static void adaptValueAtPosition(ModifiableMugenDefScript* tScript, BufferPointer writePosition, const std::string& tValue, const char* tGroupName, const char* tVariableName) {
	auto startPosition = writePosition;
	const auto line = readLineOrEOFFromTextStreamBufferPointer(&writePosition, tScript->mOwnedBuffer);
	const auto equalPos = line.find('=');
	if (equalPos == line.npos) {
		logWarningFormat("Unable to write value %s to [%s] %s. Ignoring.", tValue.c_str(), tGroupName, tVariableName);
		return;
	}

	auto start = equalPos + 1;
	while (start < line.size() && isEmptyCharacter(line[start])) start++;

	auto end = start;
	std::string previousValue;
	while (end < line.size() && line[end] != ';') {
		previousValue.push_back(line[end]);
		end++;
	}

	replaceStringInModifiableMugenDefScript(tScript, startPosition + start, tValue, previousValue.size());
}

static void writeNewVariableLine(ModifiableMugenDefScript* tScript, BufferPointer writePosition, const char* tVariableName, const std::string& tValue) {
	insertStringIntoModifiableMugenDefScript(tScript, writePosition, std::string("") + tVariableName + " = " + tValue + "\n");
}

void saveMugenDefString(ModifiableMugenDefScript* tScript, const char* tGroupName, const char* tVariableName, const std::string& tValue) {
	auto sectionStart = findExistingVariablePositionOrNullIfNonExistant(tScript, tGroupName, tVariableName);
	if (sectionStart) {
		adaptValueAtPosition(tScript, sectionStart, tValue, tGroupName, tVariableName);
	}
	else {
		if (!hasModifiableMugenDefScriptGroup(tScript, tGroupName)) {
			addModifiableMugenDefScriptGroup(tScript, tGroupName);
		}

		auto writePosition = findStartOfGroup(tScript, tGroupName);
		assert(writePosition);
		writeNewVariableLine(tScript, writePosition, tVariableName, tValue);
	}
}

void saveMugenDefFloat(ModifiableMugenDefScript* tScript, const char* tGroupName, const char* tVariableName, double tValue)
{
	std::stringstream ss;
	ss << tValue;
	saveMugenDefString(tScript, tGroupName, tVariableName, ss.str());
}

void saveMugenDefInteger(ModifiableMugenDefScript* tScript, const char* tGroupName, const char* tVariableName, int tValue) {
	std::stringstream ss;
	ss << tValue;
	saveMugenDefString(tScript, tGroupName, tVariableName, ss.str());
}