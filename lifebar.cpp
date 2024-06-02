#include "prism/lifebar.h"

#include "prism/mugenanimationhandler.h"
#include "prism/math.h"

using namespace std;

struct PrismLifeBarElement {
    int mID;
    MugenSpriteFile* mSprites;
    MugenAnimationHandlerElement* mBGAnimation;
    MugenAnimationHandlerElement* mFGAnimation;
    int mValue;
    int mMaxValue;
};

static struct {
    int mNextID = 0;
    std::map<int, PrismLifeBarElement> mList;
} gPrismLifeBarData;

static void updateLifebarDisplay(PrismLifeBarElement* e) {
    const auto ratio = (double)e->mValue / e->mMaxValue;
    const auto spriteSize = getAnimationFirstElementSpriteSize(e->mFGAnimation->mAnimation, e->mSprites);
    setMugenAnimationRectangleWidth(e->mFGAnimation, spriteSize.x * ratio);
}

int addPrismLifebar(int tEntityID, const Vector3D& tPosition, const Vector2D& tFGOffset, MugenSpriteFile* tSprites, MugenAnimations* tAnimations, int tBGAnimation, int tFGAnimation, int tStartValue, int tMaxValue)
{
    int id = gPrismLifeBarData.mNextID++;
    PrismLifeBarElement e;
    e.mID = id;
    e.mSprites = tSprites;
    e.mBGAnimation = addMugenAnimation(getMugenAnimation(tAnimations, tBGAnimation), tSprites, tPosition);
    e.mFGAnimation = addMugenAnimation(getMugenAnimation(tAnimations, tFGAnimation), tSprites, tPosition + tFGOffset);
    e.mValue = tStartValue;
    e.mMaxValue = tMaxValue;
    gPrismLifeBarData.mList[id] = e;

    updateLifebarDisplay(&gPrismLifeBarData.mList[id]);

    return id;
}

int getPrismLifebarValue(int tID)
{
    return gPrismLifeBarData.mList[tID].mValue;
}

void setPrismLifebarValue(int tID, int tValue)
{
    auto e = &gPrismLifeBarData.mList[tID];
    e->mValue = min(e->mMaxValue, max(0, tValue));
    updateLifebarDisplay(e);
}

static void loadPrismLifebarHandler(void* tData) {
    (void)tData;
    gPrismLifeBarData.mList.clear();
}

static void unloadPrismLifebarHandler(void* tData) {
    (void)tData;

    for (auto& e : gPrismLifeBarData.mList) {
        removeMugenAnimation(e.second.mBGAnimation);
        removeMugenAnimation(e.second.mFGAnimation);
    }

    gPrismLifeBarData.mList.clear();
}

ActorBlueprint getPrismLifebarHandler()
{
    return makeActorBlueprint(loadPrismLifebarHandler, unloadPrismLifebarHandler);
}