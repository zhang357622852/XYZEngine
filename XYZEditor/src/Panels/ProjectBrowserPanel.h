#pragma once
#include <XYZ.h>




namespace XYZ {
	class ProjectBrowserPanel
	{
	public:
		ProjectBrowserPanel();

		bool OnInGuiRender();



	private:
		enum
		{
			FOLDER = InGuiRenderConfiguration::DOCKSPACE + 1,
			SPRITE,
			TEXTURE,
			MATERIAL,
			BACK_ARROW
		};

		uint32_t m_PathLength;
		uint32_t m_DirectoryPathLength;
		char m_ProjectPath[260]; // This is windows max path value

		const uint32_t s_ProjectPanelID = 7;
	};
}