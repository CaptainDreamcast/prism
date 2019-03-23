#include <prism/debug.h>

#include <prism/mugentexthandler.h>
#include <prism/system.h>
#include <prism/input.h>
#include <prism/stlutil.h>


#define PREVIOUS_FPS_AMOUNT 5
#define CONSOLE_Z 90
#define CONSOLE_ARCHIVE_AMOUNT 5

using namespace std;

typedef struct {
	double mPreviousFPS[PREVIOUS_FPS_AMOUNT];
	uint64_t mPrevTime;
	int mFPSCounterTextID;

	uint64_t mStartDrawingTime;
	uint64_t mEndDrawingTime;
	int mDrawingTimeCounterTextID;

	uint64_t mStartUpdateTime;
	uint64_t mPreviousStartUpdateTime;
	uint64_t mEndUpdateTime;

	int mUpdateTimeCounterTextID;

	uint64_t mStartWaitingTime;
	uint64_t mEndWaitingTime;
	int mWaitingTimeCounterTextID;
} SideDisplay;

typedef struct {
	void* mCaller;
	string(*mCB)(void*, string);
} ConsoleCommand;

typedef struct {
	int mBackgroundAnimationID;
	string mConsoleArchiveText[CONSOLE_ARCHIVE_AMOUNT];
	int mConsoleArchiveTextID[CONSOLE_ARCHIVE_AMOUNT];
	int mArchivePointer;
	string mConsoleText;
	int mConsoleTextID;
	int mConsolePointerAnimationID;
	int mPointerPosition;
	TextureData mWhiteTexture;

	map<string, ConsoleCommand> mConsoleCommands;

	int mIsVisible;
} Console;

static struct {

	SideDisplay mSideDisplay;
	Console mConsole;

	int mIsActive;

} gPrismDebug;

void initDebug()
{
	addMugenFont(-2, "f6x9.fnt");
}

static void loadPrismDebug(void* tData) {
	(void)tData;

	ScreenSize sz = getScreenSize();
	double offset = (sz.y / 480.0) * 20;
	int dy = 10;

	gPrismDebug.mSideDisplay.mPrevTime = getSystemTicks();
	gPrismDebug.mSideDisplay.mFPSCounterTextID = addMugenTextMugenStyle("00.0", makePosition(sz.x - offset, offset, 95), makeVector3DI(-1, 1, -1));

	gPrismDebug.mSideDisplay.mStartDrawingTime = gPrismDebug.mSideDisplay.mEndDrawingTime = getSystemTicks();
	gPrismDebug.mSideDisplay.mDrawingTimeCounterTextID = addMugenTextMugenStyle("000", makePosition(sz.x - offset, offset + dy, 95), makeVector3DI(-1, 1, -1));

	gPrismDebug.mSideDisplay.mStartUpdateTime = gPrismDebug.mSideDisplay.mEndUpdateTime = getSystemTicks();
	gPrismDebug.mSideDisplay.mPreviousStartUpdateTime = getSystemTicks();
	gPrismDebug.mSideDisplay.mUpdateTimeCounterTextID = addMugenTextMugenStyle("000", makePosition(sz.x - offset, offset + dy * 2, 95), makeVector3DI(-1, 1, -1));

	gPrismDebug.mSideDisplay.mStartWaitingTime = gPrismDebug.mSideDisplay.mEndWaitingTime = getSystemTicks();
	gPrismDebug.mSideDisplay.mWaitingTimeCounterTextID = addMugenTextMugenStyle("000", makePosition(sz.x - offset, offset + dy * 3, 95), makeVector3DI(-1, 1, -1));

	gPrismDebug.mConsole.mWhiteTexture = createWhiteTexture();
	gPrismDebug.mConsole.mIsVisible = 0;

	gPrismDebug.mIsActive = 1;
}

static void unloadPrismDebug(void* tData) {
	(void)tData;
}

static void updatePrismDebugSideDisplay() {
	uint64_t currentTime = getSystemTicks();

	uint64_t dt = currentTime - gPrismDebug.mSideDisplay.mPrevTime;
	double ds = dt / 1000.0;
	double fps = 1 / ds;

	double fpsSum = fps + gPrismDebug.mSideDisplay.mPreviousFPS[PREVIOUS_FPS_AMOUNT - 1];
	for (int i = 1; i < PREVIOUS_FPS_AMOUNT; i++) {
		fpsSum += gPrismDebug.mSideDisplay.mPreviousFPS[i - 1];
		gPrismDebug.mSideDisplay.mPreviousFPS[i - 1] = gPrismDebug.mSideDisplay.mPreviousFPS[i];
	}
	gPrismDebug.mSideDisplay.mPreviousFPS[PREVIOUS_FPS_AMOUNT - 1] = fps;
	fpsSum /= PREVIOUS_FPS_AMOUNT + 1;

	char text[200];
	sprintf(text, "%.1f", fpsSum);
	changeMugenText(gPrismDebug.mSideDisplay.mFPSCounterTextID, text);
	gPrismDebug.mSideDisplay.mPrevTime = currentTime;

	sprintf(text, "%lu", (unsigned long)(gPrismDebug.mSideDisplay.mEndDrawingTime - gPrismDebug.mSideDisplay.mStartDrawingTime));
	changeMugenText(gPrismDebug.mSideDisplay.mDrawingTimeCounterTextID, text);

	sprintf(text, "%lu", (unsigned long)(gPrismDebug.mSideDisplay.mEndUpdateTime - gPrismDebug.mSideDisplay.mPreviousStartUpdateTime));
	changeMugenText(gPrismDebug.mSideDisplay.mUpdateTimeCounterTextID, text);

	sprintf(text, "%lu", (unsigned long)(gPrismDebug.mSideDisplay.mEndWaitingTime - gPrismDebug.mSideDisplay.mStartWaitingTime));
	changeMugenText(gPrismDebug.mSideDisplay.mWaitingTimeCounterTextID, text);
}

static void addConsoleText(string text) {
	for (int i = CONSOLE_ARCHIVE_AMOUNT - 1; i > 0; i--) {
		gPrismDebug.mConsole.mConsoleArchiveText[i] = gPrismDebug.mConsole.mConsoleArchiveText[i - 1];
	}
	gPrismDebug.mConsole.mConsoleArchiveText[0] = text;

	for (int i = 0; i < CONSOLE_ARCHIVE_AMOUNT; i++) {
		changeMugenText(gPrismDebug.mConsole.mConsoleArchiveTextID[i], gPrismDebug.mConsole.mConsoleArchiveText[i].data());
	}
}

static void updateConsoleText()
{
	double offset;
	if (!gPrismDebug.mConsole.mPointerPosition) {
		offset = 0;
	}
	else {
		string prefix = gPrismDebug.mConsole.mConsoleText.substr(0, gPrismDebug.mConsole.mPointerPosition);
		changeMugenText(gPrismDebug.mConsole.mConsoleTextID, prefix.data());
		offset = getMugenTextSizeX(gPrismDebug.mConsole.mConsoleTextID) + 1;
	}
	changeMugenText(gPrismDebug.mConsole.mConsoleTextID, gPrismDebug.mConsole.mConsoleText.data());
	setAnimationPosition(gPrismDebug.mConsole.mConsolePointerAnimationID, makePosition(20 + offset, 90, CONSOLE_Z + 1));
}

static void clearConsoleInput() {
	gPrismDebug.mConsole.mArchivePointer = -1;
	gPrismDebug.mConsole.mPointerPosition = 0;
	gPrismDebug.mConsole.mConsoleText.clear();
	updateConsoleText();
}

static void parseConsoleText() {
	if (!gPrismDebug.mConsole.mConsoleText.size()) return;

	string firstWord = gPrismDebug.mConsole.mConsoleText.substr(0, gPrismDebug.mConsole.mConsoleText.find(' '));

	auto it = gPrismDebug.mConsole.mConsoleCommands.find(firstWord);
	if (it != gPrismDebug.mConsole.mConsoleCommands.end()) {
		auto command = it->second;
		auto response = command.mCB(command.mCaller, gPrismDebug.mConsole.mConsoleText);
		if (response.size()) {
			addConsoleText(response);
		}
	}
	else {
		addConsoleText("Unknown command: " + firstWord);
	}
}

static void submitConsoleText() {
	addConsoleText(gPrismDebug.mConsole.mConsoleText);
	parseConsoleText();
	clearConsoleInput();
}

static void keyboardInputReceived(void* tCaller, KeyboardKeyPrism tKey) {
	(void)tCaller;

	if (tKey >= KEYBOARD_A_PRISM && tKey <= KEYBOARD_Z_PRISM) {
		char c = 'a' + (tKey - KEYBOARD_A_PRISM);
		if (hasPressedRawKeyboardKey(KEYBOARD_SHIFT_LEFT_PRISM)) c += 'A' - 'a';
		gPrismDebug.mConsole.mConsoleText.insert(gPrismDebug.mConsole.mPointerPosition, 1, c);
		gPrismDebug.mConsole.mPointerPosition++;
		updateConsoleText();
	} else if (tKey == KEYBOARD_SPACE_PRISM) {
		gPrismDebug.mConsole.mConsoleText.insert(gPrismDebug.mConsole.mPointerPosition, 1, ' ');
		gPrismDebug.mConsole.mPointerPosition++;
		updateConsoleText();
	}
	else if (tKey == KEYBOARD_PERIOD_PRISM) {
		gPrismDebug.mConsole.mConsoleText.insert(gPrismDebug.mConsole.mPointerPosition, 1, '.');
		gPrismDebug.mConsole.mPointerPosition++;
		updateConsoleText();
	}
	else if (tKey == KEYBOARD_SLASH_PRISM) {
		gPrismDebug.mConsole.mConsoleText.insert(gPrismDebug.mConsole.mPointerPosition, 1, '/');
		gPrismDebug.mConsole.mPointerPosition++;
		updateConsoleText();
	}
	else if (tKey == KEYBOARD_BACKSPACE_PRISM) {
		if (gPrismDebug.mConsole.mPointerPosition > 0) {
			gPrismDebug.mConsole.mConsoleText.erase(gPrismDebug.mConsole.mPointerPosition - 1, 1);
			gPrismDebug.mConsole.mPointerPosition--;
			updateConsoleText();
		}
	}
	else if (tKey == KEYBOARD_DELETE_PRISM) {
		if (gPrismDebug.mConsole.mPointerPosition < (int)gPrismDebug.mConsole.mConsoleText.size()) {
			gPrismDebug.mConsole.mConsoleText.erase(gPrismDebug.mConsole.mPointerPosition, 1);
			updateConsoleText();
		}
	}
	else if (tKey == KEYBOARD_LEFT_PRISM) {
		if (gPrismDebug.mConsole.mPointerPosition > 0) {
			gPrismDebug.mConsole.mPointerPosition--;
			updateConsoleText();
		}
	}
	else if (tKey == KEYBOARD_RIGHT_PRISM) {
		if (gPrismDebug.mConsole.mPointerPosition < (int)gPrismDebug.mConsole.mConsoleText.size()) {
			gPrismDebug.mConsole.mPointerPosition++;
			updateConsoleText();
		}
	}
	else if (tKey == KEYBOARD_UP_PRISM) {
		if (gPrismDebug.mConsole.mArchivePointer < CONSOLE_ARCHIVE_AMOUNT - 1) {
			gPrismDebug.mConsole.mArchivePointer++;
			gPrismDebug.mConsole.mConsoleText = gPrismDebug.mConsole.mConsoleArchiveText[gPrismDebug.mConsole.mArchivePointer];
			gPrismDebug.mConsole.mPointerPosition = gPrismDebug.mConsole.mConsoleText.size();
			updateConsoleText();
		}
	}
	else if (tKey == KEYBOARD_DOWN_PRISM) {
		if (gPrismDebug.mConsole.mArchivePointer > 0) {
			gPrismDebug.mConsole.mArchivePointer--;
			gPrismDebug.mConsole.mConsoleText = gPrismDebug.mConsole.mConsoleArchiveText[gPrismDebug.mConsole.mArchivePointer];
			gPrismDebug.mConsole.mPointerPosition = gPrismDebug.mConsole.mConsoleText.size();
			updateConsoleText();
		}
	}
	else if (tKey == KEYBOARD_RETURN_PRISM) {
		submitConsoleText();
	}

	waitForButtonFromUserInputForKeyboard(0, keyboardInputReceived);
}

static void setConsoleVisible() {
	gPrismDebug.mConsole.mBackgroundAnimationID = playOneFrameAnimationLoop(makePosition(0, 0, CONSOLE_Z), &gPrismDebug.mConsole.mWhiteTexture);
	ScreenSize sz = getScreenSize();
	setAnimationSize(gPrismDebug.mConsole.mBackgroundAnimationID, makePosition(sz.x, 100, 1), makePosition(0, 0, 0));
	setAnimationColor(gPrismDebug.mConsole.mBackgroundAnimationID, 0.3, 0.3, 0.3);
	setAnimationTransparency(gPrismDebug.mConsole.mBackgroundAnimationID, 0.7);

	for (int i = 0; i < CONSOLE_ARCHIVE_AMOUNT; i++)
	{
		gPrismDebug.mConsole.mConsoleArchiveTextID[i] = addMugenTextMugenStyle(gPrismDebug.mConsole.mConsoleArchiveText[i].data(), makePosition(20, 75 - 10 * i, CONSOLE_Z + 1), makeVector3DI(-2, 4, 1));
	}

	gPrismDebug.mConsole.mConsoleTextID = addMugenTextMugenStyle(gPrismDebug.mConsole.mConsoleText.data(), makePosition(20, 91, CONSOLE_Z + 1), makeVector3DI(-2, 0, 1));

	gPrismDebug.mConsole.mConsolePointerAnimationID = playOneFrameAnimationLoop(makePosition(20, 90, CONSOLE_Z + 1), &gPrismDebug.mConsole.mWhiteTexture);
	setAnimationSize(gPrismDebug.mConsole.mConsolePointerAnimationID, makePosition(6, 1, 1), makePosition(0, 0, 0));
	setAnimationColor(gPrismDebug.mConsole.mConsolePointerAnimationID, 1, 1, 1);
	updateConsoleText();

	waitForButtonFromUserInputForKeyboard(0, keyboardInputReceived);

	gPrismDebug.mConsole.mArchivePointer = -1;
	gPrismDebug.mConsole.mIsVisible = 1;
}

static void setConsoleInvisible() {
	removeHandledAnimation(gPrismDebug.mConsole.mBackgroundAnimationID);

	for (int i = 0; i < CONSOLE_ARCHIVE_AMOUNT; i++)
	{
		removeMugenText(gPrismDebug.mConsole.mConsoleArchiveTextID[i]);
	}

	removeMugenText(gPrismDebug.mConsole.mConsoleTextID);
	removeHandledAnimation(gPrismDebug.mConsole.mConsolePointerAnimationID);

	cancelWaitingForButtonFromUserInput(0);

	gPrismDebug.mConsole.mIsVisible = 0;
}

static void toggleVisibility() {
	if (gPrismDebug.mConsole.mIsVisible) {
		setConsoleInvisible();
	}
	else {
		setConsoleVisible();
	}
}

static void captureFlankInput() {
	hasPressedAFlank();
	hasPressedBFlank();
	hasPressedXFlank();
	hasPressedYFlank();
	hasPressedLFlank();
	hasPressedRFlank();
	hasPressedLeftFlank();
	hasPressedRightFlank();
	hasPressedUpFlank();
	hasPressedDownFlank();
	hasPressedStartFlank();
	hasShotGunFlank();
}

static void updatePrismDebugConsole() {
	if (hasPressedKeyboardKeyFlank(KEYBOARD_CARET_PRISM)) {
		toggleVisibility();
	}

	if (gPrismDebug.mConsole.mIsVisible) {
		captureFlankInput();
	}
}

static void updatePrismDebug(void* tData) {
	(void)tData;

	updatePrismDebugSideDisplay();
	updatePrismDebugConsole();

}

ActorBlueprint getPrismDebug()
{
	return makeActorBlueprint(loadPrismDebug, unloadPrismDebug, updatePrismDebug);
}

void setPrismDebugUpdateStartTime()
{
	if (!gPrismDebug.mIsActive) return;
	gPrismDebug.mSideDisplay.mEndDrawingTime = getSystemTicks();
	gPrismDebug.mSideDisplay.mPreviousStartUpdateTime = gPrismDebug.mSideDisplay.mStartUpdateTime;
	gPrismDebug.mSideDisplay.mStartUpdateTime = getSystemTicks();
}

void setPrismDebugDrawingStartTime()
{
	if (!gPrismDebug.mIsActive) return;
	gPrismDebug.mSideDisplay.mEndWaitingTime = getSystemTicks();
	gPrismDebug.mSideDisplay.mStartDrawingTime = getSystemTicks();
}

void setPrismDebugWaitingStartTime()
{
	if (!gPrismDebug.mIsActive) return;
	gPrismDebug.mSideDisplay.mEndUpdateTime = getSystemTicks();
	gPrismDebug.mSideDisplay.mStartWaitingTime = getSystemTicks();
}

void addPrismDebugConsoleCommand(std::string tCommand, std::string(*tCB)(void *tCaller, std::string tCommandInput), void* tCaller)
{
	ConsoleCommand e;
	e.mCaller = tCaller;
	e.mCB = tCB;
	gPrismDebug.mConsole.mConsoleCommands[tCommand] = e;
}
