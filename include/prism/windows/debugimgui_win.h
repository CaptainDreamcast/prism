#pragma once

#include <SDL.h>
#include <string>
#include <vector>
#include <map>
#include <imgui/imgui.h>

bool isImguiPrismActive();

void imguiPrismInitAfterDrawingSetup();

void imguiPrismProcessEvent(SDL_Event* tEvent);

void imguiPrismStartFrame();

void imguiPrismRenderStart();
void imguiPrismRenderEnd();

void imguiPrismShutdown();

void imguiPrismAddTab(const std::string_view& tTabName, const std::string_view& tEntryName, bool* tBool);

template<class K>
void imguiVector(const std::string_view& tName, std::vector<K>& tVector, void(*tElementFunction)(K& tElement))
{
	if (ImGui::TreeNode(tName.data()))
	{
		for (size_t i = 0; i < tVector.size(); i++)
		{
			if (ImGui::TreeNode(std::to_string(i).c_str()))
			{
				tElementFunction(tVector[i]);
				ImGui::TreePop();
			}
		}
		ImGui::TreePop();
	}
}

template<class K>
void imguiVector(const std::string_view& tName, std::vector<K*>& tVector, void(*tElementFunction)(K& tElement))
{
	if (ImGui::TreeNode(tName.data()))
	{
		for (size_t i = 0; i < tVector.size(); i++)
		{
			if (ImGui::TreeNode(std::to_string(i).c_str()))
			{
				tElementFunction(*tVector[i]);
				ImGui::TreePop();
			}
		}
		ImGui::TreePop();
	}
}

template<class V, class C>
void imguiIntMap(const std::string_view& tName, std::map<int, V>& tMap, void(*tElementFunction)(V& tElement, C& tCaller), C& tCaller)
{
	if (ImGui::TreeNode(tName.data()))
	{
		for (auto& elementPair : tMap)
		{
			if (ImGui::TreeNode(std::to_string(elementPair.first).c_str()))
			{
				tElementFunction(elementPair.second, tCaller);
				ImGui::TreePop();
			}
		}
		ImGui::TreePop();
	}
}

template<class V>
void imguiIntMap(const std::string_view& tName, std::map<int, V>& tMap, void(*tElementFunction)(V& tElement))
{
	if (ImGui::TreeNode(tName.data()))
	{
		for (auto& elementPair : tMap)
		{
			if (ImGui::TreeNode(std::to_string(elementPair.first).c_str()))
			{
				tElementFunction(elementPair.second);
				ImGui::TreePop();
			}
		}
		ImGui::TreePop();
	}
}


