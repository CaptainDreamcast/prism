#include "prism/windows/debugimgui_texteditor_win.h"

#include <map>
#include <fstream>
#include <imgui_texteditor/imgui_texteditor.h>

#include <prism/stlutil.h>
#include <prism/filereader.h>

namespace prism::imgui {

	struct TextEditorElement
	{
		bool isActive;
		imgui_texteditor::TextEditor editor;
	};

	static struct
	{
		std::map<std::string, TextEditorElement> mTextEditors;
	} gTextEditorHandlerData;

	void initTextEditorHandler() {}
	void updateTextEditorHandler() {
		auto it = gTextEditorHandlerData.mTextEditors.begin();
		while (it != gTextEditorHandlerData.mTextEditors.end())
		{
			if (!it->second.isActive)
			{
				it = gTextEditorHandlerData.mTextEditors.erase(it);
			}
			else
			{
				it++;
			}
		}
	}
    
	static void renderSingleTextEditor(const std::string& path, TextEditorElement& e)
	{
        ImGui::Begin(path.c_str(), &e.isActive, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar);
        ImGui::SetWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Save", "Ctrl-S"))
                {
                    if (e.editor.CanSave())
                    {
                        e.editor.Save();
                    }
                }
                if (ImGui::MenuItem("Quit", "Alt-F4"))
                {
                    e.isActive = false;
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Edit"))
            {
                bool ro = e.editor.IsReadOnly();
                if (ImGui::MenuItem("Read-only mode", nullptr, &ro))
                    e.editor.SetReadOnly(ro);
                ImGui::Separator();

                if (ImGui::MenuItem("Undo", "ALT-Backspace", nullptr, !ro && e.editor.CanUndo()))
                    e.editor.Undo();
                if (ImGui::MenuItem("Redo", "Ctrl-Y", nullptr, !ro && e.editor.CanRedo()))
                    e.editor.Redo();

                ImGui::Separator();

                if (ImGui::MenuItem("Copy", "Ctrl-C", nullptr, e.editor.HasSelection()))
                    e.editor.Copy();
                if (ImGui::MenuItem("Cut", "Ctrl-X", nullptr, !ro && e.editor.HasSelection()))
                    e.editor.Cut();
                if (ImGui::MenuItem("Delete", "Del", nullptr, !ro && e.editor.HasSelection()))
                    e.editor.Delete();
                if (ImGui::MenuItem("Paste", "Ctrl-V", nullptr, !ro && ImGui::GetClipboardText() != nullptr))
                    e.editor.Paste();

                ImGui::Separator();

                if (ImGui::MenuItem("Select all", nullptr, nullptr))
                    e.editor.SetSelection(imgui_texteditor::TextEditor::Coordinates(), imgui_texteditor::TextEditor::Coordinates(e.editor.GetTotalLines(), 0));

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("View"))
            {
                if (ImGui::MenuItem("Dark palette"))
                    e.editor.SetPalette(imgui_texteditor::TextEditor::GetDarkPalette());
                if (ImGui::MenuItem("Light palette"))
                    e.editor.SetPalette(imgui_texteditor::TextEditor::GetLightPalette());
                if (ImGui::MenuItem("Retro blue palette"))
                    e.editor.SetPalette(imgui_texteditor::TextEditor::GetRetroBluePalette());
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        ImGui::Text("%6d lines  | %s | %s | %s | %s", e.editor.GetTotalLines(),
            e.editor.IsOverwrite() ? "Ovr" : "Ins",
            e.editor.CanUndo() ? "*" : " ",
            e.editor.GetLanguageDefinition().mName.c_str(), path.c_str());

        e.editor.Render("TextEditor");

        ImGui::End();
	}

	void renderTextEditorHandler() 
	{
		for (auto& editorPair : gTextEditorHandlerData.mTextEditors)
		{
			renderSingleTextEditor(editorPair.first, editorPair.second);
		}
	}

    void openTextEditor(const std::string& path) {
        if (gTextEditorHandlerData.mTextEditors.find(path) != gTextEditorHandlerData.mTextEditors.end()) return;

        auto& editor = gTextEditorHandlerData.mTextEditors[path];
        editor.isActive = true;

        if (isFile(path))
        {
            char fullPath[1024];
            getFullPath(fullPath, path.c_str());
            std::ifstream t(fullPath);
            if (t.good())
            {
                std::string str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
                editor.editor.SetText(str);
            }
            t.close();
            editor.editor.SetPath(fullPath);
        }
    }
}