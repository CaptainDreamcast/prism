#include "prism/drawing.h"

#include <algorithm>

#include <stdlib.h>
#include <math.h>
#include <assert.h>


#include <kos.h>
#include <dc/matrix3d.h>

#include "prism/log.h"
#include "prism/datastructures.h"
#include "prism/memoryhandler.h"
#include "prism/system.h"
#include "prism/math.h"

using namespace std;

static struct {

	double a;
	double r;
	double g;
	double b;

	Vector mMatrixStack;

	BlendType mBlendType;
	int mIsDisabled;

	uint64_t mPreviousFrameTime;
	uint64_t mCurrentFrameTime;

    Vector3DI mInverted;
	
} gData;

semaphore_t gPVRAccessSemaphore;

static void applyDrawingMatrix(pvr_vertex_t* tVert) {
  (void) tVert;
  mat_trans_single3(tVert->x, tVert->y, tVert->z);
}

static void forceSingleValueToInteger(float* tVal) {
	*tVal = floor(*tVal);
}

static void forceToInteger(pvr_vertex_t* tVert) {
  
  forceSingleValueToInteger(&tVert->x);
  forceSingleValueToInteger(&tVert->y);
}


void initDrawing(){
	logg("Initiate drawing.");
	pvr_set_pal_format(PVR_PAL_ARGB8888);
	setDrawingParametersToIdentity();
	gData.mMatrixStack = new_vector();
	gData.mIsDisabled = 0;

	gData.mPreviousFrameTime = getSystemTicks();
	gData.mCurrentFrameTime = getSystemTicks();

	sem_init(&gPVRAccessSemaphore, 1);
}

#define PVR_BLEND_ZERO          0   /**< \brief None of this color */
#define PVR_BLEND_ONE           1   /**< \brief All of this color */
#define PVR_BLEND_DESTCOLOR     2   /**< \brief Destination color */
#define PVR_BLEND_INVDESTCOLOR  3   /**< \brief Inverse of destination color */
#define PVR_BLEND_SRCALPHA      4   /**< \brief Blend with source alpha */
#define PVR_BLEND_INVSRCALPHA   5   /**< \brief Blend with inverse source alpha */
#define PVR_BLEND_DESTALPHA     6   /**< \brief Blend with destination alpha */
#define PVR_BLEND_INVDESTALPHA  7   /**< \brief Blend with inverse destination alpha */

static void sendSpriteToPVR(TextureData tTexture, Rectangle tTexturePosition, pvr_vertex_t* vert) {
  //sem_wait(&gPVRAccessSemaphore);

  referenceTextureMemory(tTexture.mTexture);

  pvr_poly_cxt_t cxt;
  pvr_poly_hdr_t hdr;

  uint32_t format;
  if(tTexture.mHasPalette) {
	format = (PVR_TXRFMT_PAL8BPP | PVR_TXRFMT_8BPP_PAL(tTexture.mPaletteID));
  } else {
	format = PVR_TXRFMT_ARGB4444;
  }

  pvr_poly_cxt_txr(&cxt, PVR_LIST_TR_POLY, format, tTexture.mTextureSize.x, tTexture.mTextureSize.y, tTexture.mTexture->mData, PVR_FILTER_NEAREST);

    cxt.blend.src_enable = PVR_BLEND_DISABLE;
    cxt.blend.dst_enable = PVR_BLEND_DISABLE;

	switch(gData.mBlendType) {
		case BLEND_TYPE_NORMAL:
			cxt.blend.src = PVR_BLEND_SRCALPHA; 
			cxt.blend.dst = PVR_BLEND_INVSRCALPHA;
			break;
		case BLEND_TYPE_ADDITION:
			cxt.blend.src = PVR_BLEND_SRCALPHA; 
			cxt.blend.dst = PVR_BLEND_ONE;
			break;
		case BLEND_TYPE_SUBTRACTION:
			cxt.blend.src = PVR_BLEND_SRCALPHA; 
			cxt.blend.dst = PVR_BLEND_ONE;
			break;
		default:
			logError("Unrecognized blend type.");
			logErrorInteger(gData.mBlendType);
			abortSystem();
			break;
	}

  pvr_poly_compile(&hdr, &cxt);
  pvr_prim(&hdr, sizeof(hdr));

  double left, right, up, down;
  if(tTexturePosition.topLeft.x < tTexturePosition.bottomRight.x) {
	left = tTexturePosition.topLeft.x / ((double) tTexture.mTextureSize.x);
    right = (tTexturePosition.bottomRight.x + 1) / ((double)tTexture.mTextureSize.x);  
  } else {
	left = (tTexturePosition.topLeft.x + 1) / ((double) tTexture.mTextureSize.x);
    right = tTexturePosition.bottomRight.x / ((double)tTexture.mTextureSize.x);
  }

  if(tTexturePosition.topLeft.y < tTexturePosition.bottomRight.y) {
	up = tTexturePosition.topLeft.y / ((double) tTexture.mTextureSize.y);
    	down = (tTexturePosition.bottomRight.y + 1) / ((double)tTexture.mTextureSize.y);
  } else {
	up = (tTexturePosition.topLeft.y + 1) / ((double) tTexture.mTextureSize.y);
    	down = tTexturePosition.bottomRight.y / ((double)tTexture.mTextureSize.y);
  }

  vert[0].argb = PVR_PACK_COLOR(gData.a, gData.r, gData.g, gData.b);
  vert[0].oargb = 0;
  vert[0].flags = PVR_CMD_VERTEX;
  vert[0].u = left;
  vert[0].v = up;

  vert[1].argb = vert[0].argb;
  vert[1].oargb = 0;
  vert[1].flags = PVR_CMD_VERTEX;
  vert[1].u = right;
  vert[1].v = up;

  vert[2].argb = vert[0].argb;
  vert[2].oargb = 0;
  vert[2].flags = PVR_CMD_VERTEX;
  vert[2].u = left;
  vert[2].v = down;

 vert[3].argb = vert[0].argb;
  vert[3].oargb = 0;
  vert[3].flags = PVR_CMD_VERTEX_EOL;
  vert[3].u = right;
  vert[3].v = down;

  pvr_prim(&vert[0], sizeof(pvr_vertex_t));
  if(gData.mInverted.x ^ gData.mInverted.y) {
      pvr_prim(&vert[2], sizeof(pvr_vertex_t));
      pvr_prim(&vert[1], sizeof(pvr_vertex_t));
  } else {
      pvr_prim(&vert[1], sizeof(pvr_vertex_t));
      pvr_prim(&vert[2], sizeof(pvr_vertex_t));
  } 
  pvr_prim(&vert[3], sizeof(pvr_vertex_t));

  //sem_signal(&gPVRAccessSemaphore);
}

void drawSprite(TextureData tTexture, Position tPos, Rectangle tTexturePosition) {
  if(gData.mIsDisabled) return;

  debugLog("Draw Sprite");
  debugInteger(tTexture.mTextureSize.x);
  debugInteger(tTexture.mTextureSize.y);

  if (tTexture.mTextureSize.x < 0 || tTexture.mTextureSize.y < 0 || tTexture.mTextureSize.x % 2 != 0 || tTexture.mTextureSize.y % 2 != 0) {
    logError("Called with invalid textureSize");
    logErrorInteger(tTexture.mTextureSize.x);
    logErrorInteger(tTexture.mTextureSize.y);

    return;
  }

  int sizeX = abs(tTexturePosition.bottomRight.x - tTexturePosition.topLeft.x) + 1;
  int sizeY = abs(tTexturePosition.bottomRight.y - tTexturePosition.topLeft.y) + 1;

  pvr_vertex_t vert[4];
  
  vert[0].x = tPos.x;
  vert[0].y = tPos.y;
  vert[0].z = tPos.z;
  applyDrawingMatrix(&vert[0]);
  forceToInteger(&vert[0]);
  

  vert[1].x = tPos.x + sizeX;
  vert[1].y = tPos.y;
  vert[1].z = tPos.z;
  applyDrawingMatrix(&vert[1]);
  forceToInteger(&vert[1]);

  vert[2].x = tPos.x;
  vert[2].y = tPos.y + sizeY;
  vert[2].z = tPos.z;
  applyDrawingMatrix(&vert[2]);
  forceToInteger(&vert[2]);

  vert[3].x = tPos.x + sizeX;
  vert[3].y = tPos.y + sizeY;
  vert[3].z = tPos.z;
  applyDrawingMatrix(&vert[3]);
  forceToInteger(&vert[3]);

  double minX = min(vert[0].x, min(vert[1].x, min(vert[2].x, vert[3].x)));
  double maxX = max(vert[0].x, max(vert[1].x, max(vert[2].x, vert[3].x)));
  double minY = min(vert[0].y, min(vert[1].y, min(vert[2].y, vert[3].y)));
  double maxY = max(vert[0].y, max(vert[1].y, max(vert[2].y, vert[3].y)));

  ScreenSize sz = getScreenSize();
  if(maxX < 0) return;
  if(minX >= sz.x) return;
  if(maxY < 0) return;
  if(minY >= sz.y) return;

  sendSpriteToPVR(tTexture, tTexturePosition, vert);
}

void startDrawing() {

  //sem_wait(&gPVRAccessSemaphore);
  pvr_scene_begin();
  pvr_list_begin(PVR_LIST_TR_POLY);
  //sem_signal(&gPVRAccessSemaphore);
}

void stopDrawing() {
  //sem_wait(&gPVRAccessSemaphore);
  pvr_list_finish();
  pvr_scene_finish();
  //sem_signal(&gPVRAccessSemaphore);
}

void disableDrawing() {
	gData.mIsDisabled = 1;
}
void enableDrawing() {
	gData.mIsDisabled = 0;
}

void waitForScreen() {
  debugLog("Wait for screen");
  pvr_wait_ready();
  debugLog("Wait for screen done");

  gData.mPreviousFrameTime = gData.mCurrentFrameTime;
  gData.mCurrentFrameTime = getSystemTicks();
}

extern void getRGBFromColor(Color tColor, double* tR, double* tG, double* tB);

// TODO: refactor into general drawing code so both have it
static int hasToLinebreak(char* tText, int tCurrent, Position tTopLeft, Position tPos, Vector3D tFontSize, Vector3D tBreakSize, Vector3D tTextBoxSize) {
	
	if (tText[0] == ' ') return 0;
	if (tText[0] == '\n') return 1;
	
	char word[1024];
	int positionsRead;
	sscanf(tText + tCurrent, "%1023s%n", word, &positionsRead);

	Position delta = makePosition(positionsRead * tFontSize.x + (positionsRead-1) * tBreakSize.x, 0, 0);
	Position after = vecAdd(tPos, delta);
	Position bottomRight = vecAdd(tTopLeft, tTextBoxSize);

	return (after.x > bottomRight.x);
}

void drawMultilineText(char* tText, char* tFullText, Position tPosition, Vector3D tFontSize, Color tColor, Vector3D tBreakSize, Vector3D tTextBoxSize) {
  if(gData.mIsDisabled) return;

  //sem_wait(&gPVRAccessSemaphore);

  pvr_poly_cxt_t cxt;
  pvr_poly_hdr_t hdr;
  pvr_vertex_t vert;

  int current = 0;
  Position pos = tPosition;

  double r, g, b;
  getRGBFromColor(tColor, &r, &g, &b);

  TextureData fontData = getFontTexture();
  referenceTextureMemory(fontData.mTexture);

  pvr_poly_cxt_txr(&cxt, PVR_LIST_TR_POLY, PVR_TXRFMT_ARGB4444, fontData.mTextureSize.x, fontData.mTextureSize.y, fontData.mTexture->mData, PVR_FILTER_BILINEAR);

  pvr_poly_compile(&hdr, &cxt);
  pvr_prim(&hdr, sizeof(hdr));

  while (tText[current] != '\0') {

    FontCharacterData charData = getFontCharacterData(tText[current]);

    vert.argb = PVR_PACK_COLOR(1.0f, r, g, b);
    vert.oargb = 0;
    vert.flags = PVR_CMD_VERTEX;

    vert.x = pos.x;
    vert.y = pos.y;
    vert.z = pos.z;
    vert.u = charData.mFilePositionX1;
    vert.v = charData.mFilePositionY1;
    pvr_prim(&vert, sizeof(vert));

    vert.x = pos.x + tFontSize.x;
    vert.y = pos.y;
    vert.z = pos.z;
    vert.u = charData.mFilePositionX2;
    vert.v = charData.mFilePositionY1;
    pvr_prim(&vert, sizeof(vert));

    vert.x = pos.x;
    vert.y = pos.y + tFontSize.y;
    vert.z = pos.z;
    vert.u = charData.mFilePositionX1;
    vert.v = charData.mFilePositionY2;
    pvr_prim(&vert, sizeof(vert));

    vert.x = pos.x + tFontSize.x;
    vert.y = pos.y + tFontSize.y;
    vert.z = pos.z;
    vert.u = charData.mFilePositionX2;
    vert.v = charData.mFilePositionY2;
    vert.flags = PVR_CMD_VERTEX_EOL;
    pvr_prim(&vert, sizeof(vert));

    pos.x += tFontSize.x + tBreakSize.x;
    current++;

    if(hasToLinebreak(tFullText, current, tPosition, pos, tFontSize, tBreakSize, tTextBoxSize)) {
	pos.x = tPosition.x - (tFontSize.x + tBreakSize.x);
	pos.y += tFontSize.y + tBreakSize.y;
    }
  }

  //sem_signal(&gPVRAccessSemaphore);
}

void drawTruetypeText(char* tText, TruetypeFont tFont, Position tPosition, Vector3DI tTextSize, Vector3D tColor, double tTextBoxWidth) {
	// TODO
	(void) tText;
	(void) tFont;
	(void) tPosition;
	(void) tTextSize;
	(void) tColor;
	(void) tTextBoxWidth;
}

void scaleDrawing(double tFactor, Position tScalePosition){
	mat_translate(tScalePosition.x, tScalePosition.y, tScalePosition.z);
  	mat_scale(tFactor, tFactor, 1);
	mat_translate(-tScalePosition.x, -tScalePosition.y, -tScalePosition.z);
    gData.mInverted.x ^= tFactor < 0;
    gData.mInverted.y ^= tFactor < 0;
}

void scaleDrawing3D(Vector3D tFactor, Position tScalePosition){
	mat_translate(tScalePosition.x, tScalePosition.y, tScalePosition.z);
  	mat_scale(tFactor.x, tFactor.y, tFactor.z);
	mat_translate(-tScalePosition.x, -tScalePosition.y, -tScalePosition.z);
    gData.mInverted.x ^= tFactor.x < 0;
    gData.mInverted.y ^= tFactor.y < 0;
}

void setDrawingBaseColor(Color tColor){
	getRGBFromColor(tColor, &gData.r, &gData.g, &gData.b);
}

void setDrawingBaseColorAdvanced(double r, double g, double b) {
	gData.r = r;
	gData.g = g;
	gData.b = b;
}


void setDrawingTransparency(double tAlpha){
	gData.a = tAlpha;
}

void setDrawingBlendType(BlendType tBlendType) {
	gData.mBlendType = tBlendType;
}


void setDrawingRotationZ(double tAngle, Position tPosition){
	mat_translate(tPosition.x, tPosition.y, tPosition.z);
	mat_rotate_z(tAngle);
	mat_translate(-tPosition.x, -tPosition.y, -tPosition.z);
}

void setDrawingParametersToIdentity(){
	mat_identity();
	setDrawingBaseColor(COLOR_WHITE);
	setDrawingTransparency(1.0);
	gData.mBlendType = BLEND_TYPE_NORMAL;
    gData.mInverted.x = gData.mInverted.y = 0;
}

static void pushMatrixInternal() {
	matrix_t* mat = (matrix_t*)allocMemory(sizeof(matrix_t));
	mat_store(mat);
	vector_push_back_owned(&gData.mMatrixStack, mat);
}

static void popMatrixInternal() {
	matrix_t* mat = (matrix_t*)vector_get_back(&gData.mMatrixStack);
	mat_load(mat);
	vector_pop_back(&gData.mMatrixStack);
}

void pushDrawingTranslation(Vector3D tTranslation) {
	pushMatrixInternal();
	mat_translate(tTranslation.x, tTranslation.y, tTranslation.z);
}

void pushDrawingRotationZ(double tAngle, Vector3D tCenter) {
	pushMatrixInternal();

	mat_translate(tCenter.x, tCenter.y, tCenter.z);
	mat_rotate_z(tAngle);
	mat_translate(-tCenter.x, -tCenter.y, -tCenter.z);
}


void popDrawingRotationZ() {
	popMatrixInternal();
}

void popDrawingTranslation() {
	popMatrixInternal();
}

void drawColoredRectangleToTexture(TextureData tDst, Color tColor, Rectangle tTarget) {
	(void) tDst;
	(void) tColor;
	(void) tTarget;
}

static uint32_t packPaletteEntry(uint8_t a, uint8_t r, uint8_t g, uint8_t b) {
	return ((a) << 24) | ((r) << 16) | ((g) << 8) | ((b) << 0);
}

void setPaletteFromARGB256Buffer(int tPaletteID, Buffer tBuffer) {
	assert(tBuffer.mLength >= 256*4);
	//sem_wait(&gPVRAccessSemaphore);

	uint8_t* src = (uint8_t*)tBuffer.mData;
	
	int start = tPaletteID * 256;
	int i;
	for(i = 0; i < 256; i++) {
		uint8_t a = src[i*4+0];
		uint8_t r = src[i*4+1];
		uint8_t g = src[i*4+2];
		uint8_t b = src[i*4+3];
		uint32_t value = packPaletteEntry(a, r, g, b);
		pvr_set_pal_entry(start + i, value);
	}

	//sem_signal(&gPVRAccessSemaphore);
}

void setPaletteFromBGR256WithFirstValueTransparentBuffer(int tPaletteID, Buffer tBuffer) {
	assert(tBuffer.mLength >= 256*3);
	//sem_wait(&gPVRAccessSemaphore);

	uint8_t* src = (uint8_t*)tBuffer.mData;
	
	int start = tPaletteID * 256;
	pvr_set_pal_entry(start, 0);

	int i;
	for(i = 1; i < 256; i++) {
		uint8_t r = src[i*3+0];
		uint8_t g = src[i*3+1];
		uint8_t b = src[i*3+2];
		uint8_t a = 0xFF;
		uint32_t value = packPaletteEntry(a, r, g, b);
		pvr_set_pal_entry(start + i, value);
	}

	//sem_signal(&gPVRAccessSemaphore);
}

double getRealFramerate() {
	uint64_t delta = gData.mCurrentFrameTime - gData.mPreviousFrameTime;
	return 1000.0 / delta;
}
