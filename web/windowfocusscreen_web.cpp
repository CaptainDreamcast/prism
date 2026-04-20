#include "prism/windowfocusscreen.h"

#include <emscripten.h>

#include "prism/input.h"
#include "prism/drawing.h"
#include "prism/mugentexthandler.h"
#include "prism/wrapper.h"

namespace prism
{
    static struct
    {
        Screen* mNext;
    } gWindowFocusScreenData;

    class WindowFocusScreen
    {
    public:
        bool mPreviousAbortState = false;

        WindowFocusScreen() {
            mPreviousAbortState = getWrapperAbortEnabled();
            setWrapperAbortEnabled(false);

            auto t = addMugenTextMugenStyle("Please click", Vector3D(160, 80, 1), Vector3DI(-1, 0, 0));
            setMugenTextScale(t, 3.0);
            t = addMugenTextMugenStyle("on the screen", Vector3D(160, 100, 1), Vector3DI(-1, 0, 0));
            setMugenTextScale(t, 3.0);
            t = addMugenTextMugenStyle("to focus the window", Vector3D(160, 120, 1), Vector3DI(-1, 0, 0));
            setMugenTextScale(t, 3.0);

            t = addMugenTextMugenStyle("(and yourself)", Vector3D(160, 160, 1), Vector3DI(-1, 0, 0));
            setMugenTextScale(t, 3.0);
            t = addMugenTextMugenStyle("ommmmmmmmmmmmmmmm", Vector3D(160, 180, 1), Vector3DI(-1, 0, 0));
            setMugenTextScale(t, 3.0);
        }

        void update() {
            if (hasPressedMouseLeftFlank())
            {
                setWrapperAbortEnabled(mPreviousAbortState);
                setNewScreen(gWindowFocusScreenData.mNext);
            }
        }

    };
    EXPORT_SCREEN_CLASS(WindowFocusScreen);

    void setWindowFocusScreenNextScreen(Screen* tNext)
    {
        gWindowFocusScreenData.mNext = tNext;
    }
}