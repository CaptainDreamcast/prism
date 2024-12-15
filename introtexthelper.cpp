#include "prism/introtexthelper.h"

#include "prism/mugenanimationhandler.h"

namespace prism {

    struct IntroText
    {
        int mID;
        MugenAnimationHandlerElement* mAnimationElement;
        Vector2DI mSFX;
        MugenSounds* mSounds;
        int mDurationLeft;
        int hasPlayedSound;
    };

    static struct {
        int mNextID;
        std::map<int, IntroText> mList;
    } gPrismIntroTextHelper;

    static void loadIntroTextHelper(void*)
    {
        gPrismIntroTextHelper.mNextID = 0;
        gPrismIntroTextHelper.mList.clear();
    }

    static void unloadIntroTextHelper(void*)
    {
        for (auto& e : gPrismIntroTextHelper.mList)
        {
            removeMugenAnimation(e.second.mAnimationElement);
        }

        gPrismIntroTextHelper.mList.clear();
    }

    static int updateIntroText(void*, IntroText& e)
    {
        if (!e.hasPlayedSound) {
            tryPlayMugenSound(e.mSounds, e.mSFX.x, e.mSFX.y);
            e.hasPlayedSound = 1;
        }

        e.mDurationLeft--;
        if (e.mDurationLeft <= 0) {
            removeMugenAnimation(e.mAnimationElement);
            return 1;
        }

        return 0;
    }

    static void updateIntroTextHelper(void*)
    {
        stl_int_map_remove_predicate(gPrismIntroTextHelper.mList, updateIntroText);
    }

    ActorBlueprint getPrismIntroTextHandler()
    {
        return makeActorBlueprint(loadIntroTextHelper, unloadIntroTextHelper, updateIntroTextHelper);
    }

    int addPrismIntroText(const Position& tPosition, int tAnimation, const Vector2DI& tSFX, int tDuration, MugenSpriteFile* tSprites, MugenAnimations* tAnimations, MugenSounds* tSounds)
    {
        int id = gPrismIntroTextHelper.mNextID++;
        IntroText e;
        e.mID = id;
        e.mAnimationElement = addMugenAnimation(getMugenAnimation(tAnimations, tAnimation), tSprites, tPosition);
        e.mSFX = tSFX;
        e.mSounds = tSounds;
        e.mDurationLeft = tDuration;
        e.hasPlayedSound = 0;
        gPrismIntroTextHelper.mList[id] = e;

        return id;
    }

    int hasPrismIntroTextFinished(int tID)
    {
        return (gPrismIntroTextHelper.mList.find(tID) == gPrismIntroTextHelper.mList.end());
    }
}