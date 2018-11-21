#include "prism/errorscreen.h"

#include "prism/mugentexthandler.h"
#include "prism/datastructures.h"
#include "prism/log.h"
#include "prism/screeneffect.h"
#include "prism/input.h"

static Color getColorFromLogEntryType(LogEntry* e) {
	if (e->mType == LOG_TYPE_ERROR) return COLOR_RED;
	else if (e->mType == LOG_TYPE_WARNING) return COLOR_YELLOW;
	else return COLOR_WHITE;

}

static void loadLogEntryText() {
	Vector logEntries = getLogEntries();

	int id = addMugenText("An error has occured. Log output:", makePosition(20, 20, 1), -1);
	setMugenTextColor(id, COLOR_RED);

	double y = 40;
	int i;
	for (i = 0; i < vector_size(&logEntries); i++) {
		LogEntry* entry = vector_get(&logEntries, i);
		id = addMugenText(strchr(entry->mText, ']') + 2, makePosition(20, y, 1), -1);
		setMugenTextColor(id, getColorFromLogEntryType(entry));
		y += 10;
	}

	delete_vector(&logEntries);
}

static void loadErrorScreen() {
	setScreenBackgroundColorRGB(0, 0, 0);
	loadLogEntryText();
}

static void updateErrorScreen() {
	if (hasPressedStartFlank()) {
		gotoNextScreenAfterWrapperError();
	}
}

Screen ErrorScreen = {
	.mLoad = loadErrorScreen,
	.mUpdate = updateErrorScreen,
};