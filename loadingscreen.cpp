#include "prism/loadingscreen.h"

#include "prism/thread.h"
#include "prism/drawing.h"
#include "prism/mugentexthandler.h"
#include "prism/system.h"
#include "prism/screeneffect.h"

namespace prism {

#ifdef DREAMCAST

#define MAX_DOT_COUNT 10

	static struct {
		int mTicks;
	} gPrismLoadingScreenData;

	extern semaphore_t gPVRAccessSemaphore; // seems like it might cause issues if bios text is drawn while loading textures (though I don't get why it would)

	static void drawLoadingText() {
		const auto sz = getScreenSize();
		char text[10 + MAX_DOT_COUNT];
		strcpy(text, "Loading");
		int pos = strlen(text);
		int i;
		for (i = 0; i < gPrismLoadingScreenData.mTicks; i++) text[pos++] = '.';
		for (;i < MAX_DOT_COUNT; i++) text[pos++] = ' ';
		text[pos] = '\0';
		const auto vramWriteLocation = 20 * sz.x + 20;
		bfont_draw_str(vram_s + vramWriteLocation, sz.x, 1, text);
	}

	static void loadScreenLoop() {
		waitForScreen();
		sem_wait(&gPVRAccessSemaphore);
		drawLoadingText();
		sem_signal(&gPVRAccessSemaphore);
		gPrismLoadingScreenData.mTicks = (gPrismLoadingScreenData.mTicks + 1) % MAX_DOT_COUNT;
	}


	void startLoadingScreen(int* tHasFinishedLoadingReference)
	{
		bfont_set_encoding(BFONT_CODE_ISO8859_1);
		setScreenBackgroundColorRGB(0, 0, 0);
		gPrismLoadingScreenData.mTicks = 0;
		while (!(*tHasFinishedLoadingReference)) {
			loadScreenLoop();
		}
	}
#else

	void startLoadingScreen(int* /*tHasFinishedLoadingReference*/)
	{

	}

#endif

}