#include "stdafx.h"
#include "Lights2DInspector.h"

#include "Editor/EditorHelper.h"
#include "XYZ/Scene/Components.h"
#include "SceneTagInspector.h"


namespace XYZ {
	namespace Editor {
		SceneTagInspector::SceneTagInspector()
			:
			Inspectable("SceneTagInspector")
		{
		}
		bool SceneTagInspector::OnEditorRender()
		{
			return EditorHelper::DrawComponent<SceneTagComponent>("Scene Tag", m_Context, [&](auto& component) {

				std::string& tag = m_Context.GetComponent<SceneTagComponent>().Name;
				char buffer[256];
				memset(buffer, 0, sizeof(buffer));
				std::strncpy(buffer, tag.c_str(), sizeof(buffer));
				if (ImGui::InputText("##Tag", buffer, sizeof(buffer)))
				{
					tag = std::string(buffer);
				}
			});
		}
		void SceneTagInspector::SetSceneEntity(const SceneEntity& entity)
		{
			m_Context = entity;
		}
	}
}