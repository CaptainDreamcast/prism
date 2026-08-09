#include "prism/windows/debugimgui_win.h"

#include <algorithm>
#include <fstream>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/imgui_impl_sdl2.h>
#include <imgui_texteditor/imgui_texteditor.h>
#include <GL/glew.h>

#include <prism/debug.h>
#include <prism/input.h>

#include "prism/windows/debugimgui_texteditor_win.h"

namespace prism {

    extern SDL_Window* gSDLWindow;
    extern SDL_GLContext gGLContext;

    static struct {
        bool mIsActive;
        bool mIsShowingTaskbar;
        bool mIsFrameStarted;
        imgui_texteditor::TextEditor editor;
        std::vector<std::string> mCustomTabNames;
    } gImguiPrismData;

    bool isImguiPrismActive()
    {
        return gImguiPrismData.mIsActive;
    }

    void imguiPrismInitAfterDrawingSetup()
    {
        if (isInDevelopMode())
        {
            // Setup Dear ImGui context
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO(); (void)io;
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
            io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;      // Enable Gamepad Controls

            // Setup Dear ImGui style
            ImGui::StyleColorsDark();

            // Setup Platform/Renderer backends
            ImGui_ImplSDL2_InitForOpenGL(gSDLWindow, gGLContext);
            ImGui_ImplOpenGL3_Init();

            prism::imgui::initTextEditorHandler();

            gImguiPrismData.mIsShowingTaskbar = false;
            gImguiPrismData.mIsActive = true;
        }
    }

    void imguiPrismProcessEvent(SDL_Event* tEvent)
    {
        if (!gImguiPrismData.mIsActive) return;
        ImGui_ImplSDL2_ProcessEvent(tEvent);
    }

    void imguiPrismUpdate()
    {
        if (isInDevelopMode())
        {
            if (hasPressedKeyboardKeyFlank(KEYBOARD_ALT_LEFT_PRISM))
            {
                gImguiPrismData.mIsShowingTaskbar = !gImguiPrismData.mIsShowingTaskbar;
            }
        }

        prism::imgui::updateTextEditorHandler();
    }

    static bool testWindow = false;

    void imguiPrismStartFrame()
    {
        // StartFrame is called once per logic update, RenderEnd once per drawn frame. With time dilatation > 1 there are multiple logic updates per drawn frame, keep the already-open imgui frame instead of calling NewFrame twice without a Render in between.
        if (gImguiPrismData.mIsFrameStarted) return;
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        gImguiPrismData.mIsFrameStarted = true;
    }


    void imguiPrismRenderStart()
    {
        // With time dilatation < 1 some drawn frames have no logic update at all, so no NewFrame was called this frame, open the imgui frame here instead of asserting inside BeginMainMenuBar
        if (!gImguiPrismData.mIsFrameStarted)
        {
            imguiPrismStartFrame();
        }

        ImGuiIO& io = ImGui::GetIO(); (void)io;

        if (gImguiPrismData.mIsShowingTaskbar)
        {
            if (ImGui::BeginMainMenuBar())
            {
                if (ImGui::BeginMenu("Prism"))
                {
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Mugen"))
                {
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Screen"))
                {
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Blitz"))
                {
                    ImGui::EndMenu();
                }

                // Pre-create custom tabs registered via imguiPrismAddTab, so that later same-named BeginMenu calls merge into menus positioned before the FPS text instead of appending new menus past the right edge of the bar
                for (const auto& customTabName : gImguiPrismData.mCustomTabNames)
                {
                    if (ImGui::BeginMenu(customTabName.c_str()))
                    {
                        ImGui::EndMenu();
                    }
                }

                ImGui::SameLine(ImGui::GetWindowWidth() - 140);
                ImGui::Text("%.1f ms (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

                ImGui::EndMainMenuBar();
            }
        }

        prism::imgui::renderTextEditorHandler();
    }

    void imguiPrismRenderEnd()
    {
        if (!gImguiPrismData.mIsFrameStarted) return;
        gImguiPrismData.mIsFrameStarted = false;

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
            SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
        }
    }

    void imguiPrismShutdown()
    {
        if (!gImguiPrismData.mIsActive) return;
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
    }

    void imguiPrismAddTab(const std::string_view& tTabName, const std::string_view& tEntryName, bool* tBool)
    {
        if (!gImguiPrismData.mIsShowingTaskbar) return;

        static const char* fixedTabNames[] = { "Prism", "Mugen", "Screen", "Blitz" };
        const auto isFixedTab = std::any_of(std::begin(fixedTabNames), std::end(fixedTabNames), [&tTabName](const char* tFixedName) { return tTabName == tFixedName; });
        if (!isFixedTab && std::find(gImguiPrismData.mCustomTabNames.begin(), gImguiPrismData.mCustomTabNames.end(), tTabName) == gImguiPrismData.mCustomTabNames.end())
        {
            gImguiPrismData.mCustomTabNames.push_back(std::string(tTabName));
        }

        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu(tTabName.data()))
            {
                ImGui::MenuItem(tEntryName.data(), NULL, tBool);
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
    }
}