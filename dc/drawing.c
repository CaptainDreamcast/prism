#include "../include/drawing.h"

#include <stdlib.h>

#include <kos.h>
#include <dc/matrix3d.h>

#include "../include/log.h"

static struct {

	double a;
	double r;
	double g;
	double b;

} gData;

void applyDrawingMatrix(pvr_vertex_t* tVert) {
  (void) tVert;
  mat_trans_single(tVert->x, tVert->y, tVert->z);
}

void initDrawing(){
	logg("Initiate drawing.");
	setDrawingParametersToIdentity();
}

void drawSprite(TextureData tTexture, Position tPos, Rectangle tTexturePosition) {

  debugLog("Draw Sprite");
  debugInteger(tTexture.mTextureSize.x);
  debugInteger(tTexture.mTextureSize.y);

  if (tTexture.mTextureSize.x < 0 || tTexture.mTextureSize.y < 0 || tTexture.mTextureSize.x % 2 != 0 || tTexture.mTextureSize.y % 2 != 0) {
    logError("Called with invalid textureSize");
    logErrorInteger(tTexture.mTextureSize.x);
    logErrorInteger(tTexture.mTextureSize.y);

    return;
  }

  int sizeX = abs(tTexturePosition.bottomRight.x - tTexturePosition.topLeft.x);
  int sizeY = abs(tTexturePosition.bottomRight.y - tTexturePosition.topLeft.y);

  double left = tTexturePosition.topLeft.x / ((double) (tTexture.mTextureSize.x - 1));
  double right = tTexturePosition.bottomRight.x / ((double) (tTexture.mTextureSize.x - 1));
  double up = tTexturePosition.topLeft.y / ((double) (tTexture.mTextureSize.y - 1));
  double down = tTexturePosition.bottomRight.y / ((double) (tTexture.mTextureSize.y - 1));

  pvr_poly_cxt_t cxt;
  pvr_poly_hdr_t hdr;
  pvr_vertex_t vert;

  pvr_poly_cxt_txr(&cxt, PVR_LIST_TR_POLY, PVR_TXRFMT_ARGB4444, tTexture.mTextureSize.x, tTexture.mTextureSize.y, tTexture.mTexture, PVR_FILTER_BILINEAR);

  pvr_poly_compile(&hdr, &cxt);
  pvr_prim(&hdr, sizeof(hdr));

  vert.argb = PVR_PACK_COLOR(gData.a, gData.r, gData.g, gData.b);
  vert.oargb = 0;
  vert.flags = PVR_CMD_VERTEX;

  vert.x = tPos.x;
  vert.y = tPos.y;
  vert.z = tPos.z;
  vert.u = left;
  vert.v = up;
  applyDrawingMatrix(&vert);
  vert.z = tPos.z;
  pvr_prim(&vert, sizeof(vert));

  vert.x = tPos.x + sizeX;
  vert.y = tPos.y;
  vert.z = tPos.z;
  vert.u = right;
  vert.v = up;
  applyDrawingMatrix(&vert);
  vert.z = tPos.z;
  pvr_prim(&vert, sizeof(vert));

  vert.x = tPos.x;
  vert.y = tPos.y + sizeY;
  vert.z = tPos.z;
  vert.u = left;
  vert.v = down;
  applyDrawingMatrix(&vert);
  vert.z = tPos.z;
  pvr_prim(&vert, sizeof(vert));

  vert.x = tPos.x + sizeX;
  vert.y = tPos.y + sizeY;
  vert.z = tPos.z;
  vert.u = right;
  vert.v = down;
  vert.flags = PVR_CMD_VERTEX_EOL;
  applyDrawingMatrix(&vert);
  vert.z = tPos.z;
  pvr_prim(&vert, sizeof(vert));
}

void startDrawing() {
  pvr_scene_begin();
  pvr_list_begin(PVR_LIST_TR_POLY);
}

void stopDrawing() {
  pvr_list_finish();
  pvr_scene_finish();
}

void waitForScreen() {
  debugLog("Wait for screen");
  pvr_wait_ready();
  debugLog("Wait for screen done");
}

extern void getRGBFromColor(Color tColor, double* tR, double* tG, double* tB);

void drawText(char tText[], Position tPosition, TextSize tSize, Color tColor) {

  pvr_poly_cxt_t cxt;
  pvr_poly_hdr_t hdr;
  pvr_vertex_t vert;

  int current = 0;

  double r, g, b;
  getRGBFromColor(tColor, &r, &g, &b);

  TextureData fontData = getFontTexture();
  pvr_poly_cxt_txr(&cxt, PVR_LIST_TR_POLY, PVR_TXRFMT_ARGB4444, fontData.mTextureSize.x, fontData.mTextureSize.y, fontData.mTexture, PVR_FILTER_BILINEAR);

  pvr_poly_compile(&hdr, &cxt);
  pvr_prim(&hdr, sizeof(hdr));

  while (tText[current] != '\0') {

    FontCharacterData charData = getFontCharacterData(tText[current]);

    vert.argb = PVR_PACK_COLOR(1.0f, r, g, b);
    vert.oargb = 0;
    vert.flags = PVR_CMD_VERTEX;

    vert.x = tPosition.x;
    vert.y = tPosition.y;
    vert.z = tPosition.z;
    vert.u = charData.mFilePositionX1;
    vert.v = charData.mFilePositionY1;
    pvr_prim(&vert, sizeof(vert));

    vert.x = tPosition.x + tSize;
    vert.y = tPosition.y;
    vert.z = tPosition.z;
    vert.u = charData.mFilePositionX2;
    vert.v = charData.mFilePositionY1;
    pvr_prim(&vert, sizeof(vert));

    vert.x = tPosition.x;
    vert.y = tPosition.y + tSize;
    vert.z = tPosition.z;
    vert.u = charData.mFilePositionX1;
    vert.v = charData.mFilePositionY2;
    pvr_prim(&vert, sizeof(vert));

    vert.x = tPosition.x + tSize;
    vert.y = tPosition.y + tSize;
    vert.z = tPosition.z;
    vert.u = charData.mFilePositionX2;
    vert.v = charData.mFilePositionY2;
    vert.flags = PVR_CMD_VERTEX_EOL;
    pvr_prim(&vert, sizeof(vert));

    tPosition.x += tSize;
    current++;
  }

}

void scaleDrawing(double tFactor, Position tScalePosition){
	mat_translate(tScalePosition.x, tScalePosition.y, tScalePosition.z);
  	mat_scale(tFactor, tFactor, 1);
	mat_translate(-tScalePosition.x, -tScalePosition.y, -tScalePosition.z);
}

void scaleDrawing3D(Vector3D tFactor, Position tScalePosition){
	mat_translate(tScalePosition.x, tScalePosition.y, tScalePosition.z);
  	mat_scale(tFactor.x, tFactor.y, tFactor.z);
	mat_translate(-tScalePosition.x, -tScalePosition.y, -tScalePosition.z);
}

void setDrawingBaseColor(Color tColor){
	getRGBFromColor(tColor, &gData.r, &gData.g, &gData.b);
}

void setDrawingTransparency(double tAlpha){
	gData.a = tAlpha;
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
}
