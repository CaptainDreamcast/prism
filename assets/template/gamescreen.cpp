#include "gamescreen.h"

static struct 
{
    int mLevel = 0;
} gGameScreenData;

class GameScreen
{
    public:
    GameScreen() {
        load();
    }

    MugenSpriteFile mSprites;
    MugenAnimations mAnimations;
    MugenSounds mSounds;

    void loadFiles() {
        mSprites = loadMugenSpriteFileWithoutPalette("game/GAME.sff");
        mAnimations = loadMugenAnimationFile("game/GAME.air");
        mSounds = loadMugenSoundFile("game/GAME.snd");
    }

    void load() {
        loadFiles();
    }

    void update() {

    }
};

EXPORT_SCREEN_CLASS(GameScreen);

void resetGame()
{
    gGameScreenData.mLevel = 0;
}