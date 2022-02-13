#include "stdafx.h"
#include "TransformInspector.h"

#include "Editor/EditorHelper.h"
#include "XYZ/Scene/Components.h"

#include "XYZ/ImGui/ImGui.h"

namespace XYZ {
	namespace Editor {
		TransformInspector::TransformInspector()
			:
			Inspectable("TransformInspector")
		{
		}
		bool TransformInspector::OnEditorRender()
		{
			return EditorHelper::DrawComponent<TransformComponent>("Transform", m_Context, [&](auto& component) {

				if (ImGui::BeginTable("##TransformTable", 2, ImGuiTableFlags_SizingStretchProp))
				{
					UI::ScopedStyleStack style(true, ImGuiStyleVar_ItemSpacing, ImVec2{ 0.0f, 5.0f });
					UI::ScopedColorStack color(true,
						ImGuiCol_Button, ImVec4{ 0.5f, 0.5f, 0.5f, 1.0f },
						ImGuiCol_ButtonHovered, ImVec4{ 0.6f, 0.6f, 0.6f, 1.0f },
						ImGuiCol_ButtonActive, ImVec4{ 0.65f, 0.65f, 0.65f, 1.0f }
					);
					const float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * GImGui->Font->Scale * 2.0f;

					UI::TableRow("Translation",
						[]() { ImGui::Text("Translation"); },
						[&]() { UI::ScopedTableColumnAutoWidth scoped(3, lineHeight);
					UI::Vec3Control({ "X", "Y", "Z" }, component.Translation); }
					);

					glm::vec3 rotation = glm::degrees(component.Rotation);
					UI::TableRow("Rotation",
						[]() { ImGui::Text("Rotation"); },
						[&]() { UI::ScopedTableColumnAutoWidth scoped(3, lineHeight);
					UI::Vec3Control({ "X", "Y", "Z" }, rotation); }
					);
					component.Rotation = glm::radians(rotation);

					UI::TableRow("Scale",
						[]() { ImGui::Text("Scale"); },
						[&]() { UI::ScopedTableColumnAutoWidth scoped(3, lineHeight);
					UI::Vec3Control({ "X", "Y", "Z" }, component.Scale, 1.0f); }
					);

					ImGui::EndTable();
				}
			});
		}
		void TransformInspector::SetSceneEntity(const SceneEntity& entity)
		{
			m_Context = entity;
		}
	}
}
