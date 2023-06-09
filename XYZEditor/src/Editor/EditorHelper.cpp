#include "stdafx.h"
#include "EditorHelper.h"

#include "XYZ/ImGui/ImGui.h"

#include "EditorLayer.h"


namespace XYZ {
	namespace Editor {


		bool ButtonTransparent(const char* stringID, const glm::vec2& size, uint32_t index, bool swapX)
		{
			const auto& preferences = EditorLayer::GetData();
			const UV& buttonTexCoords = preferences.IconsSpriteSheet->GetTexCoords(index);
			if (swapX)
			{
				UV swapped = buttonTexCoords;
				std::swap(swapped[0].x, swapped[1].x);
				return UI::ImageButtonTransparent(stringID, preferences.IconsTexture->GetImage(), size,
					preferences.Color[ED::IconHoverColor], preferences.Color[ED::IconClickColor], preferences.Color[ED::IconColor],
					swapped[0],
					swapped[1]);
			}
			return UI::ImageButtonTransparent(stringID, preferences.IconsTexture->GetImage(), size,
				preferences.Color[ED::IconHoverColor], preferences.Color[ED::IconClickColor], preferences.Color[ED::IconColor],
				buttonTexCoords[0],
				buttonTexCoords[1]);	
		}

		bool Button(const char* stringID, const glm::vec2& size, uint32_t index, bool swapX)
		{
			const auto& preferences = EditorLayer::GetData();
			const UV& buttonTexCoords = preferences.IconsSpriteSheet->GetTexCoords(index);
			if (swapX)
			{
				UV swapped = buttonTexCoords;
				std::swap(swapped[0].x, swapped[1].x);
				return UI::ImageButton(stringID, preferences.IconsTexture->GetImage(), size, 0,
					ImVec4(0.1f, 0.1f, 0.1f, 1.0f),
					preferences.Color[ED::IconColor],
					swapped[0],
					swapped[1]);
			}
			return UI::ImageButton(stringID, preferences.IconsTexture->GetImage(), size, 0, ImVec4(0.1f, 0.1f, 0.1f, 1.0f),
				preferences.Color[ED::IconColor],
				buttonTexCoords[0],
				buttonTexCoords[1]);
		}

	}
	void EditorHelper::PushDisabled()
	{
		ImGuiContext& g = *GImGui;
		if ((g.CurrentItemFlags & ImGuiItemFlags_Disabled) == 0)
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, g.Style.Alpha * 0.6f);
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
	}
	void EditorHelper::PopDisabled()
	{
		ImGuiContext& g = *GImGui;
		ImGui::PopItemFlag();
		if ((g.CurrentItemFlags & ImGuiItemFlags_Disabled) == 0)
			ImGui::PopStyleVar();
	}
}
