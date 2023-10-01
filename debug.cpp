#include <prism/debug.h>

#include <prism/mugentexthandler.h>
#include <prism/system.h>
#include <prism/input.h>
#include <prism/stlutil.h>
#include <prism/wrapper.h>
#include <prism/system.h>

#define PREVIOUS_FPS_AMOUNT 5
#define CONSOLE_Z 90
#define CONSOLE_ARCHIVE_AMOUNT 5

using namespace std;

typedef struct SideDisplay_t {
	double mPreviousFPS[PREVIOUS_FPS_AMOUNT];
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

	int mIsVisible = 1;
} SideDisplay;

typedef struct {
	void* mCaller;
	string(*mCB)(void*, const std::string&);
} ConsoleCommand;

typedef struct {
	AnimationHandlerElement* mBackgroundAnimationElement;
	string mConsoleArchiveText[CONSOLE_ARCHIVE_AMOUNT];
	int mConsoleArchiveTextID[CONSOLE_ARCHIVE_AMOUNT];
	int mArchivePointer;
	string mConsoleText;
	int mConsoleTextID;
	AnimationHandlerElement* mConsolePointerAnimationElement;
	int mPointerPosition;
	TextureData mWhiteTexture;

	map<string, ConsoleCommand> mConsoleCommands;

	int mHasUserScript;
	Buffer mUserScript;
	BufferPointer mUserScriptPointer;

	int mIsVisible;
} Console;

static struct {

	SideDisplay mSideDisplay;
	Console mConsole;

	int mIsInDevelopMode;
	int mIsActive;

} gPrismDebug;

int isInDevelopMode() {
	return gPrismDebug.mIsInDevelopMode;
}

void setDevelopMode() {
	gPrismDebug.mIsInDevelopMode = 1;
}

static void toggleVisibility();

static string showConsoleCB(void* tCaller, const string& tCommand) {
	(void)tCaller;
	(void)tCommand;
	toggleVisibility();
	return "";
}

static void unloadUserScript() {
	freeBuffer(gPrismDebug.mConsole.mUserScript);
	gPrismDebug.mConsole.mHasUserScript = 0;
}

static string abortscriptCB(void* tCaller, const string& tCommand) {
	(void)tCaller;
	(void)tCommand;
	if (gPrismDebug.mConsole.mHasUserScript) {
		unloadUserScript();
	}
	return "";
}

static string exitCB(void* tCaller, const string& tCommand) {
	(void)tCaller;
	(void)tCommand;
	abortScreenHandling();
	return "";
}

static void initDebugScript() {
	gPrismDebug.mConsole.mHasUserScript = isFile("debug/user.cfg");
	if (!gPrismDebug.mConsole.mHasUserScript) return;

	gPrismDebug.mConsole.mUserScript = fileToBuffer("debug/user.cfg");
	gPrismDebug.mConsole.mUserScriptPointer = getBufferPointer(gPrismDebug.mConsole.mUserScript);
	addPrismDebugConsoleCommand("showconsole", showConsoleCB);
	addPrismDebugConsoleCommand("abortscript", abortscriptCB);
	addPrismDebugConsoleCommand("exit", exitCB);
}

void initDebug()
{
	addMugenFont(-2, "f6x9.fnt");
	initDebugScript();
}

static void loadPrismDebug(void* tData) {
	(void)tData;
	setProfilingSectionMarkerCurrentFunction();

	ScreenSize sz = getScreenSize();
	double offset = (sz.y / 480.0) * 20;
	int dy = 10;

	gPrismDebug.mSideDisplay.mFPSCounterTextID = addMugenTextMugenStyle("00.0", Vector3D(sz.x - offset, offset, 95), Vector3DI(-1, 1, -1));

	gPrismDebug.mSideDisplay.mStartDrawingTime = gPrismDebug.mSideDisplay.mEndDrawingTime = getSystemTicks();
	gPrismDebug.mSideDisplay.mDrawingTimeCounterTextID = addMugenTextMugenStyle("000", Vector3D(sz.x - offset, offset + dy, 95), Vector3DI(-1, 1, -1));

	gPrismDebug.mSideDisplay.mStartUpdateTime = gPrismDebug.mSideDisplay.mEndUpdateTime = getSystemTicks();
	gPrismDebug.mSideDisplay.mPreviousStartUpdateTime = getSystemTicks();
	gPrismDebug.mSideDisplay.mUpdateTimeCounterTextID = addMugenTextMugenStyle("000", Vector3D(sz.x - offset, offset + dy * 2, 95), Vector3DI(-1, 1, -1));

	gPrismDebug.mSideDisplay.mStartWaitingTime = gPrismDebug.mSideDisplay.mEndWaitingTime = getSystemTicks();
	gPrismDebug.mSideDisplay.mWaitingTimeCounterTextID = addMugenTextMugenStyle("000", Vector3D(sz.x - offset, offset + dy * 3, 95), Vector3DI(-1, 1, -1));

	gPrismDebug.mConsole.mWhiteTexture = createWhiteTexture();
	gPrismDebug.mConsole.mIsVisible = 0;

	gPrismDebug.mIsActive = 1;
}

static void setConsoleInvisible();

static void unloadPrismDebug(void* tData) {
	(void)tData;
	setProfilingSectionMarkerCurrentFunction();

	if (gPrismDebug.mConsole.mIsVisible) {
		setConsoleInvisible();
	}
}

static void updatePrismDebugSideDisplay() {
	double fps = getRealFramerate();
	double fpsSum = fps + gPrismDebug.mSideDisplay.mPreviousFPS[PREVIOUS_FPS_AMOUNT - 1];
	for (int i = 1; i < PREVIOUS_FPS_AMOUNT; i++) {
		fpsSum += gPrismDebug.mSideDisplay.mPreviousFPS[i - 1];
		gPrismDebug.mSideDisplay.mPreviousFPS[i - 1] = gPrismDebug.mSideDisplay.mPreviousFPS[i];
	}
	gPrismDebug.mSideDisplay.mPreviousFPS[PREVIOUS_FPS_AMOUNT - 1] = fps;
	fpsSum /= PREVIOUS_FPS_AMOUNT + 1;

	if (gPrismDebug.mSideDisplay.mIsVisible) {
		char text[200];
		sprintf(text, "%.1f", fpsSum);
		changeMugenText(gPrismDebug.mSideDisplay.mFPSCounterTextID, text);

		sprintf(text, "%lu", (unsigned long)(gPrismDebug.mSideDisplay.mEndDrawingTime - gPrismDebug.mSideDisplay.mStartDrawingTime));
		changeMugenText(gPrismDebug.mSideDisplay.mDrawingTimeCounterTextID, text);

		sprintf(text, "%lu", (unsigned long)(gPrismDebug.mSideDisplay.mEndUpdateTime - gPrismDebug.mSideDisplay.mPreviousStartUpdateTime));
		changeMugenText(gPrismDebug.mSideDisplay.mUpdateTimeCounterTextID, text);

		sprintf(text, "%lu", (unsigned long)(gPrismDebug.mSideDisplay.mEndWaitingTime - gPrismDebug.mSideDisplay.mStartWaitingTime));
		changeMugenText(gPrismDebug.mSideDisplay.mWaitingTimeCounterTextID, text);
	}
	else {
		changeMugenText(gPrismDebug.mSideDisplay.mFPSCounterTextID, "");
		changeMugenText(gPrismDebug.mSideDisplay.mDrawingTimeCounterTextID, "");
		changeMugenText(gPrismDebug.mSideDisplay.mUpdateTimeCounterTextID, "");
		changeMugenText(gPrismDebug.mSideDisplay.mWaitingTimeCounterTextID, "");
	}
}

static void addConsoleText(const string& text) {
	for (int i = CONSOLE_ARCHIVE_AMOUNT - 1; i > 0; i--) {
		gPrismDebug.mConsole.mConsoleArchiveText[i] = gPrismDebug.mConsole.mConsoleArchiveText[i - 1];
	}
	gPrismDebug.mConsole.mConsoleArchiveText[0] = text;

	if (gPrismDebug.mConsole.mIsVisible) {
		for (int i = 0; i < CONSOLE_ARCHIVE_AMOUNT; i++) {
			changeMugenText(gPrismDebug.mConsole.mConsoleArchiveTextID[i], gPrismDebug.mConsole.mConsoleArchiveText[i].data());
		}
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
	setAnimationPosition(gPrismDebug.mConsole.mConsolePointerAnimationElement, Vector3D(20 + offset, 90, CONSOLE_Z + 1));
}

static void clearConsoleInput() {
	gPrismDebug.mConsole.mArchivePointer = -1;
	gPrismDebug.mConsole.mPointerPosition = 0;
	gPrismDebug.mConsole.mConsoleText.clear();
	if (gPrismDebug.mConsole.mIsVisible) {
		updateConsoleText();
	}
}

static void parseConsoleText() {
	if (!gPrismDebug.mConsole.mConsoleText.size()) return;

	const auto firstWord = gPrismDebug.mConsole.mConsoleText.substr(0, gPrismDebug.mConsole.mConsoleText.find(' '));
	if (firstWord.size() > 0 && firstWord[0] == '#') return;

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

static void textInputReceived(void* /*tCaller*/, const std::string& tText) {
	gPrismDebug.mConsole.mConsoleText.insert(gPrismDebug.mConsole.mPointerPosition, tText);
	gPrismDebug.mConsole.mPointerPosition += int(tText.size());
	updateConsoleText();
}

static void keyboardInputReceived(void* tCaller, KeyboardKeyPrism tKey) {
	(void)tCaller;

	if (tKey == KEYBOARD_BACKSPACE_PRISM) {
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
			gPrismDebug.mConsole.mPointerPosition = int(gPrismDebug.mConsole.mConsoleText.size());
			updateConsoleText();
		}
	}
	else if (tKey == KEYBOARD_DOWN_PRISM) {
		if (gPrismDebug.mConsole.mArchivePointer > 0) {
			gPrismDebug.mConsole.mArchivePointer--;
			gPrismDebug.mConsole.mConsoleText = gPrismDebug.mConsole.mConsoleArchiveText[gPrismDebug.mConsole.mArchivePointer];
			gPrismDebug.mConsole.mPointerPosition = int(gPrismDebug.mConsole.mConsoleText.size());
			updateConsoleText();
		}
	}
	else if (tKey == KEYBOARD_RETURN_PRISM) {
		submitConsoleText();
	}

	waitForButtonFromUserInputForKeyboard(0, keyboardInputReceived);
}

static void setConsoleVisible() {
	gPrismDebug.mConsole.mBackgroundAnimationElement = playOneFrameAnimationLoop(Vector3D(0, 0, CONSOLE_Z), &gPrismDebug.mConsole.mWhiteTexture);
	ScreenSize sz = getScreenSize();
	setAnimationSize(gPrismDebug.mConsole.mBackgroundAnimationElement, Vector3D(sz.x, 100, 1), Vector3D(0, 0, 0));
	setAnimationColor(gPrismDebug.mConsole.mBackgroundAnimationElement, 0.3, 0.3, 0.3);
	setAnimationTransparency(gPrismDebug.mConsole.mBackgroundAnimationElement, 0.7);

	for (int i = 0; i < CONSOLE_ARCHIVE_AMOUNT; i++)
	{
		gPrismDebug.mConsole.mConsoleArchiveTextID[i] = addMugenTextMugenStyle(gPrismDebug.mConsole.mConsoleArchiveText[i].data(), Vector3D(20, 75 - 10 * i, CONSOLE_Z + 1), Vector3DI(-2, 4, 1));
	}

	gPrismDebug.mConsole.mConsoleTextID = addMugenTextMugenStyle(gPrismDebug.mConsole.mConsoleText.data(), Vector3D(20, 91, CONSOLE_Z + 1), Vector3DI(-2, 0, 1));

	gPrismDebug.mConsole.mConsolePointerAnimationElement = playOneFrameAnimationLoop(Vector3D(20, 90, CONSOLE_Z + 1), &gPrismDebug.mConsole.mWhiteTexture);
	setAnimationSize(gPrismDebug.mConsole.mConsolePointerAnimationElement, Vector3D(6, 1, 1), Vector3D(0, 0, 0));
	setAnimationColor(gPrismDebug.mConsole.mConsolePointerAnimationElement, 1, 1, 1);
	updateConsoleText();

	waitForCharacterFromUserInput(0, textInputReceived);
	waitForButtonFromUserInputForKeyboard(0, keyboardInputReceived);

	gPrismDebug.mConsole.mArchivePointer = -1;
	gPrismDebug.mConsole.mIsVisible = 1;
}

static void setConsoleInvisible() {
	removeHandledAnimation(gPrismDebug.mConsole.mBackgroundAnimationElement);

	for (int i = 0; i < CONSOLE_ARCHIVE_AMOUNT; i++)
	{
		removeMugenText(gPrismDebug.mConsole.mConsoleArchiveTextID[i]);
	}

	removeMugenText(gPrismDebug.mConsole.mConsoleTextID);
	removeHandledAnimation(gPrismDebug.mConsole.mConsolePointerAnimationElement);

	cancelWaitingForCharacterFromUserInput(0);
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

static void updatePrismDebugConsole() {
	if (hasPressedKeyboardKeyFlank(KEYBOARD_CARET_PRISM)) {
		toggleVisibility();
	}
}

static void updatePrismUserScript() {
	if (!gPrismDebug.mConsole.mHasUserScript) return;

	gPrismDebug.mConsole.mConsoleText = readLineOrEOFFromTextStreamBufferPointer(&gPrismDebug.mConsole.mUserScriptPointer, gPrismDebug.mConsole.mUserScript);
	if (isBufferPointerOver(gPrismDebug.mConsole.mUserScriptPointer, gPrismDebug.mConsole.mUserScript)) {
		unloadUserScript();
	}

	submitConsoleText();
}

static void updatePrismDebug(void* tData) {
	(void)tData;
	setProfilingSectionMarkerCurrentFunction();

	updatePrismDebugSideDisplay();
	updatePrismDebugConsole();
	updatePrismUserScript();
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

int getPrismDebugSideDisplayVisibility()
{
	if (!gPrismDebug.mIsActive) return 0;
	return gPrismDebug.mSideDisplay.mIsVisible;
}

void setPrismDebugSideDisplayVisibility(int tIsVisible)
{
	if (!gPrismDebug.mIsActive) return;
	gPrismDebug.mSideDisplay.mIsVisible = tIsVisible;
}

void togglePrismDebugSideDisplayVisibility()
{
	setPrismDebugSideDisplayVisibility(!getPrismDebugSideDisplayVisibility());
}

int isPrismDebugConsoleVisible()
{
	return gPrismDebug.mConsole.mIsVisible;
}

void addPrismDebugConsoleCommand(const std::string& tCommand, std::string(*tCB)(void *tCaller, const std::string& tCommandInput), void* tCaller)
{
	ConsoleCommand e;
	e.mCaller = tCaller;
	e.mCB = tCB;
	gPrismDebug.mConsole.mConsoleCommands[tCommand] = e;
}

void submitToPrismDebugConsole(const std::string& tText) {
	addConsoleText(tText);
}
