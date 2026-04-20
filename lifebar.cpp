#include "prism/lifebar.h"

#include "prism/mugenanimationhandler.h"

namespace prism {

    class LifeBar
    {
    public:
        MugenAnimationHandlerElement* mFGElement;
        MugenAnimationHandlerElement* mBGElement;
        int mCurrentValue;
        int mMaxValue;
        int mFullSize;
        LifeBarType mType;

        LifeBar(const Vector3D& tPosition, MugenSpriteFile& mSprites, MugenAnimations& mAnimations, int tFGAnimNo, int tBGAnimNo, LifeBarType tType, int tStartValue, int tMaxValue, int tFullSize, const Vector3D& tFGOffset)
        {
            mFGElement = addMugenAnimation(getMugenAnimation(&mAnimations, tFGAnimNo), &mSprites, tPosition + tFGOffset);
            mBGElement = addMugenAnimation(getMugenAnimation(&mAnimations, tBGAnimNo), &mSprites, tPosition);
            mCurrentValue = tStartValue;
            mMaxValue = tMaxValue;
            mFullSize = tFullSize;
            mType = tType;
            updateInternal();
        }
        LifeBar() {}

        void unload()
        {
            removeMugenAnimation(mFGElement);
            removeMugenAnimation(mBGElement);
        }

        void updateInternal()
        {
            double percentage = ((double)mCurrentValue) / mMaxValue;
            if (mType == LifeBarType::STRETCH)
            {
                setMugenAnimationDrawScale(mFGElement, Vector2D(percentage * mFullSize, 1));
            }
            else if (mType == LifeBarType::WIDTH)
            {
                setMugenAnimationRectangleWidth(mFGElement, int(percentage * mFullSize));
            }
        }

        void updateByPercentage(double tPercentage)
        {
            mCurrentValue = (int)(tPercentage * mMaxValue);
            updateInternal();
        }

        void updateByValue(int tValue)
        {
            mCurrentValue = tValue;
            updateInternal();
        }

        void setVisibility(int tIsVisible)
        {
            setMugenAnimationVisibility(mFGElement, tIsVisible);
            setMugenAnimationVisibility(mBGElement, tIsVisible);
        }
    };

    static struct
    {
        std::map<int, LifeBar> mLifeBars;
    } gLifeBarHandlerData;


    class LifeBarHandler {
    public:
        LifeBarHandler()
        {
            gLifeBarHandlerData.mLifeBars.clear();
        }
        ~LifeBarHandler()
        {
            gLifeBarHandlerData.mLifeBars.clear();
        }

        void update()
        {
        }
    };

    EXPORT_ACTOR_CLASS(LifeBarHandler);

    int addLifeBar(const Vector3D& tPosition, MugenSpriteFile& mSprites, MugenAnimations& mAnimations, int tFGAnimNo, int tBGAnimNo, LifeBarType tType, int tStartValue, int tMaxValue, int tFullSize, const Vector3D& tFGOffset)
    {
        int id = stl_int_map_get_id();
        gLifeBarHandlerData.mLifeBars[id] = LifeBar(tPosition, mSprites, mAnimations, tFGAnimNo, tBGAnimNo, tType, tStartValue, tMaxValue, tFullSize, tFGOffset);
        return id;
    }
    void removeLifeBar(int tID)
    {
        gLifeBarHandlerData.mLifeBars[tID].unload();
        gLifeBarHandlerData.mLifeBars.erase(tID);
    }

    int getLifeBarPercentage(int tID)
    {
        return gLifeBarHandlerData.mLifeBars[tID].mCurrentValue / gLifeBarHandlerData.mLifeBars[tID].mMaxValue;
    }
    void setLifeBarPercentage(int tID, double tPercentage)
    {
        gLifeBarHandlerData.mLifeBars[tID].updateByPercentage(tPercentage);
    }
    int getLifeBarValue(int tID)
    {
        return gLifeBarHandlerData.mLifeBars[tID].mCurrentValue;
    }
    void setLifeBarValue(int tID, int tValue)
    {
        gLifeBarHandlerData.mLifeBars[tID].updateByValue(tValue);
    }

    void setLifeBarVisibility(int tID, int tIsVisible)
    {
        gLifeBarHandlerData.mLifeBars[tID].setVisibility(tIsVisible);
    }

    void removeAllLifebars()
    {
        for (auto& lifeBar : gLifeBarHandlerData.mLifeBars)
        {
            lifeBar.second.unload();
        }
        gLifeBarHandlerData.mLifeBars.clear();
    }

}