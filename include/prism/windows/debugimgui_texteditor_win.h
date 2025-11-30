#pragma once

#include <string>

namespace prism::imgui {

void initTextEditorHandler();
void updateTextEditorHandler();
void renderTextEditorHandler();

void openTextEditor(const std::string& path);

}