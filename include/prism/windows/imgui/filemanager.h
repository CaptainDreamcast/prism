#pragma once

#include <prism/actorhandler.h>

namespace prism::imgui {

	ActorBlueprint getImguiFileManager();

	enum class FileUsageType
	{
		AUTO = 0,
		SCRIPT,
		SPRITES,
		ANIMATIONS,
		SOUNDS,
		BGM,
		UNKNOWN
	};

	void addDebugFileUsage(const char* tPath, FileUsageType tType = FileUsageType::AUTO);
	void removeDebugFileUsage(const char* tPath);
	void fileManagerImguiRender();
}