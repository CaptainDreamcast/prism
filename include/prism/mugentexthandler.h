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

void addMugenFont(int tKey, char* tPath);
void loadMugenTextHandler();
void loadMugenSystemFonts();
void loadMugenFightFonts();
void unloadMugenFonts();

extern ActorBlueprint MugenTextHandler;

int addMugenText(char* tText, Position tPosition, int tFont);
void removeMugenText(int tID);
void setMugenTextFont(int tID, int tFont);
void setMugenTextAlignment(int tID, MugenTextAlignment tAlignment);
void setMugenTextColor(int tID, Color tColor);
void setMugenTextColorRGB(int tID, double tR, double tG, double tB);
void setMugenTextRectangle(int tID, GeoRectangle tRectangle);
void setMugenTextPosition(int tID, Position tPosition);
void setMugenTextTextBoxWidth(int tID, double tWidth);
void setMugenTextBuildup(int tID, Duration mBuildUpDurationPerLetter);
void setMugenTextBuiltUp(int tID);
int isMugenTextBuiltUp(int tID);

void changeMugenText(int tID, char* tText);

Position* getMugenTextPositionReference(int tID);

MugenTextAlignment getMugenTextAlignmentFromMugenAlignmentIndex(int tIndex);
Color getMugenTextColorFromMugenTextColorIndex(int tIndex);