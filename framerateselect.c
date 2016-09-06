#include "include/framerateselectscreen.h"

#include <kos.h>

#include "include/framerate.h"
#include "include/drawing.h"
#include "include/physics.h"
#include "include/input.h"

typedef struct {
  char ArrayToBePrinted[17];
  int PositionX;
  int PositionY;
  int FontSize;
} FightMenuPointStruct;

typedef struct {
  char ArrayToBePrinted[100];
  int PositionX;
  int PositionY;
  int FontSize;
} LargerArrayStruct;

#define FRAMERATE_WARNING_ARRAY_AMOUNT 2

void setFramerateEverywhere(Framerate tFramerate, int tDM) {
  setFramerate(tFramerate);
  vid_set_mode(tDM, PM_RGB565);
}

void setFramerateSixty() {
  setFramerateEverywhere(SIXTY_HERTZ, DM_640x480_NTSC_IL);
}

void setFramerateVGA() {
  setFramerateEverywhere(SIXTY_HERTZ, DM_640x480_VGA);
}

void setFramerateFifty() {
  setFramerateEverywhere(FIFTY_HERTZ, DM_640x480_PAL_IL);
}

int hasToSetFramerate() {
  int whichVideoCable;

  whichVideoCable = vid_check_cable();

  if (whichVideoCable == CT_VGA) {
    setFramerateVGA();
  }
  else {
    if (flashrom_get_region() == FLASHROM_REGION_EUROPE) {

      return 1;

    }
    else {
      setFramerateSixty();
    }
  }
  return 0;
}

#define PAL_INDEX 0
#define NTSC_INDEX 1

FramerateSelectReturnType selectFramerate() {

//  if (!hasToSetFramerate()) {
//    return FRAMERATE_SCREEN_RETURN_NORMAL;
//  }

  FightMenuPointStruct selectableArray[FRAMERATE_AMOUNT];
  LargerArrayStruct currentOptionArray;
  LargerArrayStruct warningArray[FRAMERATE_AMOUNT];

  uint8 i;
  uint8 whichArraySelected = NTSC_INDEX;
  uint8 isRunning;

  sprintf(selectableArray[0].ArrayToBePrinted, "50HZ");
  sprintf(selectableArray[1].ArrayToBePrinted, "60HZ");
  sprintf(currentOptionArray.ArrayToBePrinted, "CURRENT SELECTED OPTION: %s", selectableArray[whichArraySelected].ArrayToBePrinted);
  sprintf(warningArray[0].ArrayToBePrinted, "WARNING: 50HZ MODE WILL RESULT IN SLOWER GAMEPLAY!!");
  sprintf(warningArray[1].ArrayToBePrinted, "USE 60HZ IF POSSIBLE!");

  for (i = 0; i < FRAMERATE_AMOUNT; i++) {
    selectableArray[i].FontSize = 50;
    selectableArray[i].PositionY = 150;
  }

  selectableArray[0].PositionX = 80;
  selectableArray[1].PositionX = 360;

  currentOptionArray.PositionX = 40;
  currentOptionArray.PositionY = 335;
  currentOptionArray.FontSize = 20;

  for (i = 0; i < FRAMERATE_WARNING_ARRAY_AMOUNT; i++) {
    warningArray[i].FontSize = 11;
  }

  warningArray[0].PositionX = 40;
  warningArray[0].PositionY = 375;

  warningArray[1].PositionX = 205;
  warningArray[1].PositionY = 390;

  setFramerateFifty();
  isRunning = 1;
  while (isRunning) {

    waitForScreen();
    startDrawing();

    for (i = 0; i < FRAMERATE_AMOUNT; i++) {
      drawText(selectableArray[i].ArrayToBePrinted, makePosition(selectableArray[i].PositionX, selectableArray[i].PositionY, 1), selectableArray[i].FontSize, COLOR_WHITE);
    }
    drawText(selectableArray[whichArraySelected].ArrayToBePrinted, makePosition(selectableArray[whichArraySelected].PositionX, selectableArray[whichArraySelected].PositionY, 2), selectableArray[whichArraySelected].FontSize, COLOR_GREEN);
    drawText(currentOptionArray.ArrayToBePrinted, makePosition(currentOptionArray.PositionX, currentOptionArray.PositionY, 1), currentOptionArray.FontSize, COLOR_WHITE);

    if (whichArraySelected == PAL_INDEX) {
      for (i = 0; i < FRAMERATE_WARNING_ARRAY_AMOUNT; i++)
        drawText(warningArray[i].ArrayToBePrinted, makePosition(warningArray[i].PositionX, warningArray[i].PositionY, 1), warningArray[i].FontSize, COLOR_RED);
    }

    stopDrawing();

    updateInput();

    if (hasPressedLeftFlank() && whichArraySelected > 0) {
      whichArraySelected--;
      sprintf(currentOptionArray.ArrayToBePrinted, "CURRENT SELECTED OPTION: %s", selectableArray[whichArraySelected].ArrayToBePrinted);
    }

    if (hasPressedRightFlank() && whichArraySelected < FRAMERATE_AMOUNT - 1) {
      whichArraySelected++;
      sprintf(currentOptionArray.ArrayToBePrinted, "CURRENT SELECTED OPTION: %s", selectableArray[whichArraySelected].ArrayToBePrinted);
    }

    if (hasPressedStartFlank() || hasPressedAFlank()) {
      if (whichArraySelected == PAL_INDEX) {
        setFramerateFifty();
      }
      else {
        setFramerateSixty();
      }
      isRunning = 0;
    }

    if (hasPressedAbortFlank()) {
      return FRAMERATE_SCREEN_RETURN_ABORT;
    }
  }

  return FRAMERATE_SCREEN_RETURN_NORMAL;
}

