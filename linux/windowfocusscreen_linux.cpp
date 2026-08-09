#include "prism/windowfocusscreen.h"

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
        WindowFocusScreen() {}
        void update() {
            setNewScreen(gWindowFocusScreenData.mNext);
        }
    };
    EXPORT_SCREEN_CLASS(WindowFocusScreen);

    void setWindowFocusScreenNextScreen(Screen* tNext)
    {
        gWindowFocusScreenData.mNext = tNext;
    }
}
