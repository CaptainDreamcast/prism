#include "titlescreen.h"

#include <prism/blitz.h>
#include <prism/soundeffect.h>

#include "storyscreen.h"
#include "gamescreen.h"

using namespace prism;

class TitleScreen
{
public:
    MugenSpriteFile mSprites;
    MugenAnimations mAnimations;
    MugenSounds mSounds;

    MugenAnimationHandlerElement* titleAnimation;

    TitleScreen() {
        mSprites = loadMugenSpriteFileWithoutPalette("game/TITLE.sff");
        mAnimations = loadMugenAnimationFile("game/TITLE.air");
        mSounds = loadMugenSoundFile("game/TITLE.snd");

        titleAnimation = addMugenAnimation(getMugenAnimation(&mAnimations, 1), &mSprites, Vector3D(0, 0, 1));
        if (isOnDreamcast())
        {
            setSoundEffectVolume(1.0);
        }
        else
        {
            setVolume(0.25);
            setSoundEffectVolume(0.5);
        }
        streamMusicFile("game/GAME.ogg");

        setWrapperTitleScreen(getTitleScreen());
    }
    void update() {
        updateInput();

    }

    void updateInput()
    {
        if(hasPressedStartFlank())
        {
            resetGame();
            tryPlayMugenSound(&mSounds, 1, 0);
            setCurrentStoryDefinitionFile("game/INTRO.def", 1);
            setNewScreen(getStoryScreen());
        }
    }
};

EXPORT_SCREEN_CLASS(TitleScreen);