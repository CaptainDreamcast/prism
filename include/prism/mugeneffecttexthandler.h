#pragma once

#include "mugentexthandler.h"

namespace prism {

ActorBlueprint getMugenEffectTextHandler();

void drawMugenEffectText(char* tText, const Position& tPosition, int tFont);
int addMugenEffectText(const char* tText, const Position& tPosition, int tFont);
int addMugenEffectTextMugenStyle(const char* tText, const Position& tPosition, const Vector3DI& tFont);
void removeMugenEffectText(int tID);
void setMugenEffectTextFont(int tID, int tFont);
void setMugenEffectTextAlignment(int tID, MugenTextAlignment tAlignment);
void setMugenEffectTextColor(int tID, Color tColor);
void setMugenEffectTextColorRGB(int tID, double tR, double tG, double tB);
void setMugenEffectTextRectangle(int tID, const GeoRectangle2D& tRectangle);
void setMugenEffectTextPosition(int tID, const Position& tPosition);
void addMugenEffectTextPosition(int tID, const Position& tPosition);
void setMugenEffectTextTextBoxWidth(int tID, double tWidth);
void setMugenEffectTextBuildup(int tID, Duration mBuildUpDurationPerLetter);
void setMugenEffectTextBuiltUp(int tID);
int isMugenEffectTextBuiltUp(int tID);
int getMugenEffectTextVisibility(int tID);
void setMugenEffectTextVisibility(int tID, int tIsVisible);
double getMugenEffectTextSizeX(int tID);
void setMugenEffectTextScale(int tID, double tScale); // only for bitmap fonts for now
double getMugenEffectTextTransparency(int tID);
void setMugenEffectTextTransparency(int tID, double tOpacity);

const char* getMugenEffectTextText(int tID);
const char* getMugenEffectTextDisplayedText(int tID);
void changeMugenEffectText(int tID, const char* tText);

Position getMugenEffectTextPosition(int tID);
Position* getMugenEffectTextPositionReference(int tID);
Vector3D getMugenEffectTextColor(int tIndex);


MugenTextAlignment getMugenEffectTextAlignmentFromMugenAlignmentIndex(int tIndex);
Color getMugenEffectTextColorFromMugenEffectTextColorIndex(int tIndex);

}