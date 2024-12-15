#include "prism/numberpopuphandler.h"

#include "prism/mugentexthandler.h"

namespace prism {

    struct NumberPopup {
        int mID;
        int mTextID;
        Vector3D mVelocity;
        int mDurationLeft;
    };

    static struct {
        int mNextID;
        std::map<int, NumberPopup> mList;
    } gNumberPopupHandler;

    static int updateSingleNumberPopup(void*, NumberPopup& e)
    {
        e.mDurationLeft--;
        if (e.mDurationLeft <= 0) {
            removeMugenText(e.mTextID);
            return 1;
        }

        addMugenTextPosition(e.mTextID, e.mVelocity);
        return 0;
    }

    static void updateNumberPopupHandler(void*)
    {
        stl_int_map_remove_predicate(gNumberPopupHandler.mList, updateSingleNumberPopup);
    }

    static void loadNumberPopupHandler(void*)
    {
        gNumberPopupHandler.mList.clear();
    }

    static void unloadNumberPopupHandler(void*)
    {
        for (auto& e : gNumberPopupHandler.mList)
        {
            removeMugenText(e.second.mTextID);
        }

        gNumberPopupHandler.mList.clear();
    }

    int addPrismNumberPopup(int tValue, const Position& tPos, int tFont, const Vector3D& tVelocity, double tScale, int tColor, int tDuration)
    {
        int id = gNumberPopupHandler.mNextID++;
        NumberPopup e;
        e.mID = id;
        e.mTextID = addMugenTextMugenStyle(std::to_string(tValue).c_str(), tPos, Vector3DI(tFont, tColor, 0));
        setMugenTextScale(e.mTextID, tScale);
        e.mVelocity = tVelocity;
        e.mDurationLeft = tDuration;
        gNumberPopupHandler.mList[id] = e;

        return id;
    }

    ActorBlueprint getPrismNumberPopupHandler()
    {
        return makeActorBlueprint(loadNumberPopupHandler, unloadNumberPopupHandler, updateNumberPopupHandler);
    }

}