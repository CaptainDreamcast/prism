#include <prism/blitzclick.h>

#include <assert.h>

#include <prism/blitzcomponent.h>
#include <prism/blitzentity.h>
#include <prism/blitzmugenanimation.h>
#include <prism/input.h>

struct ClickEntry {
    int mEntityID;
    bool mIsPassiveHitboxCheck;
    bool mIsClicked;
    bool mIsClickedReal;
    bool mIsClickedLastFrame;
};

static struct {
    std::map<int, ClickEntry> mEntries;
} gBlitzClickData;

static void unregisterEntity(int tEntityID);

static BlitzComponent getBlitzClickComponent() {
	return makeBlitzComponent(unregisterEntity);
}

static void loadBlitzClickHandler(void*) {
    gBlitzClickData.mEntries.clear();
}

static void unloadBlitzClickHandler(void*) {
    gBlitzClickData.mEntries.clear();
}

static void updateSingleBlitzClickEntryPassiveAnimationHitbox(ClickEntry& e) {
    e.mIsClickedLastFrame = e.mIsClickedReal;
    e.mIsClickedReal = isMouseLeftPressed(); // TODO: proper mouse flanks
    const auto& activeHitBoxes = getBlitzMugenAnimationActiveHitboxes(e.mEntityID);
    for (const auto& activeHitBox : activeHitBoxes) {
        const auto offset = getBlitzEntityPosition(e.mEntityID);
        const auto rectangle = activeHitBox.mCollider.mImpl.mRect;
        const auto geoRect = GeoRectangle2D(rectangle.mTopLeft.x, rectangle.mTopLeft.y, rectangle.mBottomRight.x - rectangle.mTopLeft.x, rectangle.mBottomRight.y - rectangle.mTopLeft.y) + Vector2D(offset.x, offset.y);
        if (isMouseInRectangle(geoRect)) {
            e.mIsClicked = e.mIsClickedReal && !e.mIsClickedLastFrame;
        }
        else
        {
            e.mIsClicked = false;
        }
    }
}

static void updateSingleBlitzClickEntry(ClickEntry& e) {
    if(e.mIsPassiveHitboxCheck)
    {
        updateSingleBlitzClickEntryPassiveAnimationHitbox(e);
    }
    else
    {
        assert(false && "Unimplemented blitz click type.");
    }
}

static void updateBlitzClickHandler(void*) {
    for (auto& e : gBlitzClickData.mEntries) {
        updateSingleBlitzClickEntry(e.second);
    }
}

ActorBlueprint getBlitzClickHandler(){
	return makeActorBlueprint(loadBlitzClickHandler, unloadBlitzClickHandler, updateBlitzClickHandler);
}

void addBlitzClickComponent(int tEntityID)
{
	ClickEntry e;
	e.mEntityID = tEntityID;
    e.mIsPassiveHitboxCheck = false;
	registerBlitzComponent(tEntityID, getBlitzClickComponent());
	gBlitzClickData.mEntries[tEntityID] = e;
}

void addBlitzClickComponentPassiveAnimationHitbox(int tEntityID)
{
    addBlitzClickComponent(tEntityID);
    auto& e = gBlitzClickData.mEntries[tEntityID];
    e.mIsPassiveHitboxCheck = true;
    e.mIsClicked = false;
    e.mIsClickedReal = false;
    e.mIsClickedLastFrame = false;
}

bool isBlitzEntityClicked(int tEntityID) {
    assert(gBlitzClickData.mEntries.count(tEntityID));
    return gBlitzClickData.mEntries[tEntityID].mIsClicked && !gBlitzClickData.mEntries[tEntityID].mIsClickedLastFrame;
}

static void unregisterEntity(int tEntityID) {
	gBlitzClickData.mEntries.erase(tEntityID);
}