#include "prism/windows/imgui/filemanager.h"

#include <imgui/imgui.h>

#include <prism/file.h>
#include <prism/log.h>
#include <prism/windows/debugimgui_win.h>
#include <prism/windows/debugimgui_texteditor_win.h>

namespace prism::imgui {

	static struct {
		bool isActive = false;
	} gImguiFileManagerData;

	class ImguiFileManager
	{
	public:
		struct FileEntry
		{
			int count;
			FileUsageType usageType;
		};

		std::map<std::string, FileEntry> mScripts;
		std::map<std::string, FileEntry> mSpriteFiles;
		std::map<std::string, FileEntry> mAnimationFiles;
		std::map<std::string, FileEntry> mSoundFiles;
		std::map<std::string, FileEntry> mBGMFiles;

		ImguiFileManager() {
			gImguiFileManagerData.isActive = true;
		}

		~ImguiFileManager() {
			gImguiFileManagerData.isActive = false;
		}

		void update() {}

		void imguiRenderFilesGeneral(const char* tFileListHeader, std::map<std::string, FileEntry>& fileUsageContainer) {
			ImGui::Text(tFileListHeader);
			ImGui::Separator();
			for (auto& s : fileUsageContainer)
			{
				ImGui::Text("%s", (s.first + " ").c_str());
			}
		}

		void imguiRenderFilesScript(const char* tFileListHeader) {
			ImGui::Text(tFileListHeader);
			ImGui::Separator();
			for (auto& s : mScripts)
			{
				ImGui::Text("%s", (s.first + " ").c_str()); ImGui::SameLine();
				if (ImGui::Button((std::string("Edit##") + s.first).c_str()))
				{
					openTextEditor(s.first);
				}
			}
		}

		void imguiRender() {
			static bool isWindowShown = false;
			imguiPrismAddTab("Screen", "File Manager", &isWindowShown);
			if (isWindowShown)
			{
				ImGui::Begin("File Manager", &isWindowShown);
				imguiRenderFilesScript("Script files"); ImGui::Separator();
				imguiRenderFilesGeneral("Sprite files", mSpriteFiles); ImGui::Separator();
				imguiRenderFilesScript("Animation files"); ImGui::Separator();
				imguiRenderFilesGeneral("Sound files", mSoundFiles); ImGui::Separator();
				imguiRenderFilesGeneral("BGM files", mBGMFiles);
				ImGui::End();
			}
		}

		FileUsageType determineFileType(const char* tPath, FileUsageType tType)
		{
			if (tType != FileUsageType::AUTO) return tType;
			if (!hasFileExtension(tPath)) return FileUsageType::UNKNOWN;
			const auto fileExtension = getFileExtension(tPath);
			if (!strcmp("txt", fileExtension)) return FileUsageType::SCRIPT;
			if (!strcmp("def", fileExtension)) return FileUsageType::SCRIPT;
			if (!strcmp("sff", fileExtension)) return FileUsageType::SPRITES;
			if (!strcmp("air", fileExtension)) return FileUsageType::ANIMATIONS;
			if (!strcmp("snd", fileExtension)) return FileUsageType::SOUNDS;
			if (!strcmp("ogg", fileExtension)) return FileUsageType::BGM;
			return FileUsageType::UNKNOWN;
		}

		void addGeneralFileUsage(const char* tPath, std::map<std::string, FileEntry>& fileUsageContainer, FileUsageType type)
		{
			auto it = fileUsageContainer.find(tPath);
			if (it != fileUsageContainer.end())
			{
				it->second.count++;
			}
			else
			{
				fileUsageContainer[tPath] = FileEntry{ 1, type };
			}
		}

		void addDebugFileUsage(const char* tPath, FileUsageType tType) {
			tType = determineFileType(tPath, tType);

			switch (tType)
			{
			case FileUsageType::SCRIPT:
				addGeneralFileUsage(tPath, mScripts, FileUsageType::SCRIPT);
				break;
			case FileUsageType::SPRITES:
				addGeneralFileUsage(tPath, mSpriteFiles, FileUsageType::SPRITES);
				break;
			case FileUsageType::ANIMATIONS:
				addGeneralFileUsage(tPath, mAnimationFiles, FileUsageType::ANIMATIONS);
				break;
			case FileUsageType::SOUNDS:
				addGeneralFileUsage(tPath, mSoundFiles, FileUsageType::SOUNDS);
				break;
			case FileUsageType::BGM:
				addGeneralFileUsage(tPath, mBGMFiles, FileUsageType::BGM);
				break;
			case FileUsageType::UNKNOWN:
			default:
				logErrorFormat("Unable to parse file type from: %s", tPath);
				break;
			}
		}

		void removeDebugScriptUsage(const char* tPath) {
			auto it = mScripts.find(tPath);
			if (it != mScripts.end())
			{
				it->second.count--;
				if (it->second.count <= 0) mScripts.erase(it);
			}
		}

		void removeDebugFileUsage(const char* tPath) {
			removeDebugScriptUsage(tPath);
		}
	};
	EXPORT_ACTOR_CLASS(ImguiFileManager);

	void addDebugFileUsage(const char* tPath, FileUsageType tType) {
		if (!gImguiFileManagerData.isActive) return;
		char path[1024];
		getFullPath(path, tPath);
		FILE* probe = fopen(path, "rb");
		if (!probe) return;
		fclose(probe);
		gImguiFileManager->addDebugFileUsage(tPath, tType);

	}
	void removeDebugFileUsage(const char* tPath) {
		if (!gImguiFileManagerData.isActive) return;
		gImguiFileManager->removeDebugFileUsage(tPath);
	}

	void fileManagerImguiRender()
	{
		if (!gImguiFileManagerData.isActive) return;
		gImguiFileManager->imguiRender();
	}
}