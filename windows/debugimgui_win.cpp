#include "prism/windows/debugimgui_win.h"

#include <imgui/imgui.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/imgui_impl_sdl2.h>
#include <GL/glew.h>

#include <prism/debug.h>

extern SDL_Window* gSDLWindow;
extern SDL_GLContext gGLContext;

static struct {
    bool mIsActive;

} gImguiPrismData;

bool isImguiPrismActive()
{
    return gImguiPrismData.mIsActive;
}

void imguiPrismInitAfterDrawingSetup()
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(gSDLWindow, gGLContext);
    ImGui_ImplOpenGL3_Init();

    gImguiPrismData.mIsActive = isInDevelopMode();
    gImguiPrismData.mIsActive = false;
}

void imguiPrismProcessEvent(SDL_Event* tEvent)
{
    ImGui_ImplSDL2_ProcessEvent(tEvent);
}

static bool testWindow = false;

void imguiPrismStartFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
}


void imguiPrismRenderStart()
{
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    static bool showDemoWindow = false; // TODO: remove after familiar enough with imgui?
    if (showDemoWindow)
    {
        ImGui::ShowDemoWindow(&showDemoWindow);
    }

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("Prism"))
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

        ImGui::SameLine(ImGui::GetWindowWidth() - 140);
        ImGui::Text("%.1f ms (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

        ImGui::EndMainMenuBar();
    }
}

void imguiPrismRenderEnd()
{
   
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void imguiPrismShutdown()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

void imguiPrismAddTab(const std::string_view& tTabName, const std::string_view& tEntryName, bool* tBool)
{
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