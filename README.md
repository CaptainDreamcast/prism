# libtari - Library for Dreamcast / Windows game development

A loose collection of components that are often used in games (e.g.: physics, collisions, animations, etc.). The Dreamcast part works with KallistiOS, the Windows part with SDL.  
  
The most important components are as follows:
  
## Wrapper / Screen handling
Used with `#include <tari/wrapper.h>`  
This is the easiest way to use the library. 

Small example for a main function:
```C
  setGameName("THE BEST GAME EVER");
  setScreenSize(640, 480);
  initTariWrapperWithDefaultFlags();
  setFileSystem("/pc");
  startScreenHandling(&FirstScreen);
```

Let's step through this code one by one. The first line sets the game title. This has no effect on the Sega Dreamcast and only serves to set the Window name on Windows.  
The second line sets the screen size. Since the Sega Dreamcast mainly supports 320x240 and 640x480 resolution games, these are the only options.
The third line initiates the actual library. It inits memory handling, sound and all components which are of interest on a game-wide scale.
With the fourth line, we set the file system we want to work with. KallistiOS supports a large array of filesystems such as /rd, /pc and /cd. These filesystems are automatically prepended to any file path operation, such as loading textures or reading in scripts. Windows does not use different file systems and always uses ./ in the executable folder as its prefix.
The final line starts the actual screen handling. The function expects a pointer to a Screen object. Screen objects have the following functions:
```C
void (*mLoad)();
void (*mUpdate)();
void (*mDraw)();
void (*mUnload)();
Screen* (*mGetNextScreen)();
```

The load function is called after screen-wide components (physics handler, animation handler, collision handler, etc.) have finished loading. 
The update function is called once a frame after the libtari components' update functions have been called.
The draw function is called once a frame after the libtari components' drawing functions have been called. All update functions are called before the drawing functions.
The unload function is called when screen handling is aborted or switches to another screen.
The getNextScreen function is called once per frame after drawing has concluded and is used to control the game flow. If it returns a NULL pointer, nothing happens. If it calls the abortScreenHandling() function, the program leaves the screen handler and resumes operation after the startScreenHandling function. If it returns a pointer to a screen, screen handling switches to the screen the pointer points to.  
  
## Memory handler
Used with `#include <tari/memoryhandler.h>`

Memory handling for texture memory and standard memory allocation. Works based on stacks. Memory stack entries can be pushed and popped. When a memory stack entry is popped, all memory that was allocated while the entry was on top is deallocated.  
As an example, this is the way memory handling is used during screen handling: Before the screen is loaded a new entry is pushed to the memory stack. After the screen is unloaded, the entry is popped. On the upside, this reduces memory leaks and faulty memory frees. On the flipside, it makes carrying over allocated data over multiple screens difficult. 
Standard memory allocation is used with two functions:
```C
void* allocMemory();
void freeMemory();
```
When memory handling is disabled, these two functions simply call malloc and free, respectively.

## Framerate select screen
Used with `#include <tari/framerateselectscreen.h>`

Dreamcast games run with 50 frames per second on European PAL TVs and with 60 frames per second on NTSC TVs. Usually, Dreamcast games let the player select the framerate themselves. To faciliate this, a simple select screen is included in the library.  
It is skipped for VGA cables. Windows games are set to run at 60 FPS and do not need a select screen either. Libtari components do not change their outward behaviour based on framerate, e.g. animations still have the same durations and physics still appear to have the same movement speed.  
A simple example for calling the framerate select screen is given as follows:

```C
FramerateSelectReturnType framerateReturnType = selectFramerate();
if (framerateReturnType == FRAMERATE_SCREEN_RETURN_ABORT) {
	returnToMenu();
}
```
The screen checks for the Dreamcast abort command (A+B+X+Y+START) and returns an abort return value accordingly. The usual way to react to this command is to return to the Dreamcast menu.
_Important_: It should be called after the screen size is set, but before the libtari wrapper is initialized.

## Animation Handler