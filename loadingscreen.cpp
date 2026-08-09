#include "prism/loadingscreen.h"

#include <stdlib.h>
#include <string.h>

#include "prism/thread.h"
#include "prism/drawing.h"
#include "prism/mugentexthandler.h"
#include "prism/system.h"
#include "prism/screeneffect.h"
#include "prism/log.h"

namespace prism {

#ifdef DREAMCAST

#define MAX_DOT_COUNT 10
#define LOADING_ICON_DEFAULT_MARGIN 20

	static struct {
		int mTicks;
	} gPrismLoadingScreenData;

	static struct {
		uint16_t* mPixels; // RGB565
		int mWidth;
		int mHeight;
		int mHasCustomPosition;
		int mX;
		int mY;
	} gPrismLoadingScreenIconData;

	extern semaphore_t gPVRAccessSemaphore; // seems like it might cause issues if bios text is drawn while loading textures (though I don't get why it would)

	static void unloadLoadingScreenIcon() {
		if (gPrismLoadingScreenIconData.mPixels) {
			free(gPrismLoadingScreenIconData.mPixels);
			gPrismLoadingScreenIconData.mPixels = NULL;
		}
		gPrismLoadingScreenIconData.mWidth = 0;
		gPrismLoadingScreenIconData.mHeight = 0;
	}

	void setLoadingScreenIconPng(const char* tPath) {
		unloadLoadingScreenIcon();
		if (!tPath) return;

		if (strcmp("png", getFileExtension(tPath))) {
			logWarningFormat("Loading screen icon %s is not a png file. Ignoring.", tPath);
			return;
		}
		if (!isFile(tPath)) {
			logWarningFormat("Unable to find loading screen icon %s. Ignoring.", tPath);
			return;
		}

		int width, height;
		Buffer argb32Buffer = loadPNGARGB32Buffer(tPath, &width, &height);

		const auto sz = getScreenSize();
		if (width <= 0 || height <= 0 || width > sz.x || height > sz.y) {
			logWarningFormat("Loading screen icon %s has invalid size %d/%d. Ignoring.", tPath, width, height);
			freeBuffer(argb32Buffer);
			return;
		}

		gPrismLoadingScreenIconData.mPixels = (uint16_t*)malloc(width * height * sizeof(uint16_t));
		const uint8_t* src = (const uint8_t*)argb32Buffer.mData;
		for (int i = 0; i < width * height; i++) {
			const uint32_t b = (src[i * 4 + 0] * src[i * 4 + 3]) / 255;
			const uint32_t g = (src[i * 4 + 1] * src[i * 4 + 3]) / 255;
			const uint32_t r = (src[i * 4 + 2] * src[i * 4 + 3]) / 255;
			gPrismLoadingScreenIconData.mPixels[i] = uint16_t(((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
		}
		freeBuffer(argb32Buffer);

		gPrismLoadingScreenIconData.mWidth = width;
		gPrismLoadingScreenIconData.mHeight = height;
	}

	void setLoadingScreenIconPosition(int tX, int tY) {
		gPrismLoadingScreenIconData.mHasCustomPosition = 1;
		gPrismLoadingScreenIconData.mX = tX;
		gPrismLoadingScreenIconData.mY = tY;
	}

	static void drawLoadingIcon() {
		if (!gPrismLoadingScreenIconData.mPixels) return;

		const auto sz = getScreenSize();
		const int w = gPrismLoadingScreenIconData.mWidth;
		const int h = gPrismLoadingScreenIconData.mHeight;
		int x, y;
		if (gPrismLoadingScreenIconData.mHasCustomPosition) {
			x = gPrismLoadingScreenIconData.mX;
			y = gPrismLoadingScreenIconData.mY;
		}
		else {
			x = LOADING_ICON_DEFAULT_MARGIN;
			y = sz.y - h - LOADING_ICON_DEFAULT_MARGIN;
		}
		x = (x < 0) ? 0 : ((x + w > sz.x) ? (sz.x - w) : x);
		y = (y < 0) ? 0 : ((y + h > sz.y) ? (sz.y - h) : y);

		for (int row = 0; row < h; row++) {
			memcpy(vram_s + (y + row) * sz.x + x, gPrismLoadingScreenIconData.mPixels + row * w, w * sizeof(uint16_t));
		}
	}

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
		drawLoadingIcon();
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

	void setLoadingScreenIcon(const char* /*tPath*/)
	{

	}

	void setLoadingScreenIconPosition(int /*tX*/, int /*tY*/)
	{

	}

#endif

}
