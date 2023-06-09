#pragma once
#include <XYZ/Core/Timestep.h>
#include <XYZ/Event/InputEvent.h>
#include <XYZ/Event/ApplicationEvent.h>

#include "XYZ/Renderer/OrthographicCamera.h"

namespace XYZ {
	struct OrthographicCameraBounds
	{
		float Left, Right;
		float Bottom, Top;

		float GetWidth() { return Right - Left; }
		float GetHeight() { return Top - Bottom; }
	};

	class OrthographicCameraController
	{
	public:
		OrthographicCameraController(float aspectRatio, bool rotation = false);

		void OnUpdate(Timestep ts);
		void OnEvent(Event& e);

		OrthographicCamera& GetCamera() { return m_Camera; }
		const OrthographicCamera& GetCamera() const { return m_Camera; }

		float GetZoomLevel() const { return m_ZoomLevel; }
		float GetAspectRatio() const { return m_AspectRatio; }
		void SetZoomLevel(float level) { m_ZoomLevel = level; }
		void SetAspectRatio(float aspect);

		const OrthographicCameraBounds& GetBounds() const { return m_Bounds; }
		const glm::vec3& GetPosition() const { return m_CameraPosition; }
	private:
		bool onMouseScrolled(MouseScrollEvent& e);

	private:
		float m_AspectRatio;
		float m_ZoomLevel = 1.0f;
		OrthographicCameraBounds m_Bounds;
		OrthographicCamera m_Camera;

		bool m_Rotation;

		glm::vec3 m_CameraPosition = { 0.0f, 0.0f, 0.0f };
		float m_CameraRotation = 0.0f; //In degrees, in the anti-clockwise direction
		float m_CameraTranslationSpeed = 5.0f, m_CameraRotationSpeed = 180.0f;
	};
}