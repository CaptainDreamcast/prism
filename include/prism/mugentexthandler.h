#pragma once

#include "actorhandler.h"
#include "geometry.h"
#include "drawing.h"
#include "animation.h"

typedef enum {
	MUGEN_TEXT_ALIGNMENT_LEFT,
	MUGEN_TEXT_ALIGNMENT_CENTER,
	MUGEN_TEXT_ALIGNMENT_RIGHT,
} MugenTextAlignment;

int hasMugenFont(int tKey); 
void addMugenFont(int tKey, const char* tPath); 
void loadMugenTextHandler();
void unloadMugenFonts();

int getMugenFontSizeY(int tKey);
int getMugenFontSpacingY(int tKey);

ActorBlueprint getMugenTextHandler();

void drawMugenText(char* tText, const Position& tPosition, int tFont);
int addMugenText(const char* tText, const Position& tPosition, int tFont);
int addMugenTextMugenStyle(const char* tText, const Position& tPosition, const Vector3DI& tFont);
void removeMugenText(int tID);
void setMugenTextFont(int tID, int tFont);
void setMugenTextAlignment(int tID, MugenTextAlignment tAlignment);
void setMugenTextColor(int tID, Color tColor);
void setMugenTextColorRGB(int tID, double tR, double tG, double tB);
void setMugenTextRectangle(int tID, const GeoRectangle2D& tRectangle);
void setMugenTextPosition(int tID, const Position& tPosition);
void addMugenTextPosition(int tID, const Position& tPosition);
void setMugenTextTextBoxWidth(int tID, double tWidth);
void setMugenTextBuildup(int tID, Duration mBuildUpDurationPerLetter);
void setMugenTextBuiltUp(int tID);
int isMugenTextBuiltUp(int tID);
int getMugenTextVisibility(int tID);
void setMugenTextVisibility(int tID, int tIsVisible);
double getMugenTextSizeX(int tID);
void setMugenTextScale(int tID, double tScale); // only for bitmap fonts for now

const char* getMugenTextText(int tID);
const char* getMugenTextDisplayedText(int tID);
void changeMugenText(int tID, const char* tText);

Position getMugenTextPosition(int tID);
Position* getMugenTextPositionReference(int tID);
Vector3D getMugenTextColor(int tIndex);


MugenTextAlignment getMugenTextAlignmentFromMugenAlignmentIndex(int tIndex);
Color getMugenTextColorFromMugenTextColorIndex(int tIndex);
