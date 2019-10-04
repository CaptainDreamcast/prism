#include "prism/mugenscriptparser.h"

#include "prism/memoryhandler.h"
#include "prism/log.h"
#include "prism/system.h"

typedef struct {
	int(*mIsFunc)(MugenDefScriptGroup *);
	void(*mHandleFunc)(MugenDefScriptGroup *);
} ScriptParseElement;

static struct {
	int mIsActive;
	Vector mParseElements;
} gPrismMugenScriptParserData;

static void clearMugenScriptParser() {
	delete_vector(&gPrismMugenScriptParserData.mParseElements);
	gPrismMugenScriptParserData.mIsActive = 0;
}

void resetMugenScriptParser()
{
	if (gPrismMugenScriptParserData.mIsActive) {
		clearMugenScriptParser();
	}

	gPrismMugenScriptParserData.mParseElements = new_vector();
	gPrismMugenScriptParserData.mIsActive = 1;
}

void addMugenScriptParseFunction(int(*tIsFunc)(MugenDefScriptGroup *), void(*tHandleFunc)(MugenDefScriptGroup *))
{
	ScriptParseElement* e = (ScriptParseElement*)allocMemory(sizeof(ScriptParseElement));
	e->mIsFunc = tIsFunc;
	e->mHandleFunc = tHandleFunc;
	vector_push_back_owned(&gPrismMugenScriptParserData.mParseElements, e);
}

void parseMugenScript(MugenDefScript * tScript)
{
	MugenDefScriptGroup* current = tScript->mFirstGroup;
	while (current != NULL) {
		int hasFound = 0;
		int i;
		for (i = 0; i < vector_size(&gPrismMugenScriptParserData.mParseElements); i++) {
			ScriptParseElement* e = (ScriptParseElement*)vector_get(&gPrismMugenScriptParserData.mParseElements, i);
			if (e->mIsFunc(current)) {
				e->mHandleFunc(current);
				hasFound = 1;
				break;
			}
		}

		if (!hasFound) {
			logError("Unable to find script group.");
			logErrorString(current->mName.data());
			recoverFromError();
		}

		current = current->mNext;
	}

	clearMugenScriptParser();
}
