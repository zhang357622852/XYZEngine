#include "GameLayer.h"


#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <random>

#include "vendor/vox/ogt_vox.h"

namespace XYZ {

	struct GPUTimeQueries
	{
		uint32_t GPUTime = 0;
		uint32_t Renderer2DPassQuery = 0;

		static constexpr uint32_t Count() { return sizeof(GPUTimeQueries) / sizeof(uint32_t); }
	};


	static GPUTimeQueries s_TimeQueries;

	static std::pair<float, float> GetMouseViewportSpace()
	{
		auto [mx, my] = Input::GetMousePosition();
		auto [winPosX, winPosY] = Input::GetWindowPosition();

		mx += winPosX;
		my += winPosY;

		const auto viewportWidth = Input::GetWindowSize().first;
		const auto viewportHeight = Input::GetWindowSize().second;

		return { (mx / viewportWidth) * 2.0f - 1.0f, ((my / viewportHeight) * 2.0f - 1.0f) * -1.0f };
	}
	static uint32_t RandomColor()
	{
		std::random_device rd; // obtain a random number from hardware
		std::mt19937 gen(rd()); // seed the generator
		std::uniform_int_distribution<> distr(0, 255); // define the range

		uint32_t result = 0;
		result |= static_cast<uint8_t>(distr(gen));		  // R
		result |= static_cast<uint8_t>(distr(gen)) << 8;  // G
		result |= static_cast<uint8_t>(distr(gen)) << 16; // B
		result |= static_cast<uint8_t>(255) << 24; // A
	
		return result;
	}

	static uint32_t ColorToUINT(const glm::vec4& color)
	{
		uint32_t result = 0;
		result |= static_cast<uint32_t>(color.x * 255.0f);	   // R
		result |= static_cast<uint32_t>(color.y * 255.0f) << 8;  // G
		result |= static_cast<uint32_t>(color.z * 255.0f) << 16; // B
		result |= static_cast<uint32_t>(color.w * 255.0f) << 24; // A

		return result;
	}

	static uint32_t ColorToUINT(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
	{
		uint32_t result = 0;
		result |= static_cast<uint32_t>(r);	   // R
		result |= static_cast<uint32_t>(g) << 8;  // G
		result |= static_cast<uint32_t>(b) << 16; // B
		result |= static_cast<uint32_t>(a) << 24; // A

		return result;
	}

	static uint32_t Index3D(int x, int y, int z, int width, int height)
	{
		return x + width * (y + height * z);
	}

	static Ref<RenderPass> CreateRenderPass()
	{
		FramebufferSpecification compFramebufferSpec;
		compFramebufferSpec.ClearColor = { 0.1f, 0.1f, 0.1f, 1.0f };
		compFramebufferSpec.SwapChainTarget = true;
		compFramebufferSpec.ClearOnLoad = false;

		compFramebufferSpec.Attachments = { ImageFormat::RGBA };
		Ref<Framebuffer> framebuffer = Framebuffer::Create(compFramebufferSpec);

		RenderPassSpecification renderPassSpec;
		renderPassSpec.TargetFramebuffer = framebuffer;
		return RenderPass::Create(renderPassSpec);;
	}

	static Ref<Pipeline> CreatePipeline(const Ref<Shader>& shader, const Ref<RenderPass>& renderPass)
	{
		PipelineSpecification specification;
		specification.Shader = shader;
		specification.Layouts = shader->GetLayouts();
		specification.RenderPass = renderPass;
		specification.Topology = PrimitiveTopology::Triangles;
		specification.DepthTest = true;
		return Pipeline::Create(specification);
	}

	static glm::mat4 CameraFrustum(const EditorCamera& camera)
	{
		glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);
		glm::vec3 Right = glm::vec3(1.0f, 0.0f, 0.0f);
		glm::vec3 Forward = glm::vec3(0.0f, 0.0f, 1.0f);

		const float aspect = camera.GetAspectRatio();
		const float fov = glm::tan(glm::radians(camera.GetFOV() * 0.5f));

		
		const glm::vec3 goUp = Up * fov;
		const glm::vec3 goRight = Right * fov * aspect;

		const glm::vec3 topLeft		= -Forward - goRight + goUp;
		const glm::vec3 topRight	= -Forward + goRight + goUp;
		const glm::vec3 bottomRight = -Forward + goRight - goUp;
		const glm::vec3 bottomLeft	= -Forward - goRight - goUp;

		glm::mat4 frustum(1.0f);
		frustum[0] = { topLeft.x,     topLeft.y,     topLeft.z, 0.0f };
		frustum[1] = { topRight.x,    topRight.y,    topRight.z, 0.0f };
		frustum[2] = { bottomRight.x, bottomRight.y, bottomRight.z, 0.0f };
		frustum[3] = { bottomLeft.x,  bottomLeft.y,  bottomLeft.z, 0.0f };
		return frustum;
	}

	GameLayer::GameLayer()
		:
		m_Camera(30.0f, 1.778f, 0.1f, 1000.0f)
	{
	}

	GameLayer::~GameLayer()
	{
		
	}

	void GameLayer::OnAttach()
	{
		Renderer::InitResources(false);
		Application::Get().GetImGuiLayer()->EnableDockspace(false);
		const uint32_t windowWidth = Application::Get().GetWindow().GetWidth();
		const uint32_t windowHeight = Application::Get().GetWindow().GetHeight();

	
		m_Camera.SetViewportSize((float)windowWidth, (float)windowHeight);
	
		m_CommandBuffer = PrimaryRenderCommandBuffer::Create(0, "GameCommandBuffer");
		m_CommandBuffer->CreateTimestampQueries(GPUTimeQueries::Count());
		m_RenderPass = CreateRenderPass();

		
		
		

		
		m_SceneBufferSet = UniformBufferSet::Create(3);
		m_SceneBufferSet->Create(sizeof(UBScene), 0, 0);

		setupScene();
		setupCharacter();

	
		Renderer::WaitAndRenderAll();		
	}

	void GameLayer::OnDetach()
	{
	}
	void GameLayer::OnUpdate(Timestep ts)
	{
		if (m_QueuedGenerate)
			generateVoxels();

		voxelPreview();
		m_Camera.OnUpdate(ts);

		m_CommandBuffer->Begin();

		s_TimeQueries.GPUTime = m_CommandBuffer->BeginTimestampQuery();
	
		m_SceneUB.ViewProjection = m_Camera.GetViewProjection();
		m_SceneUB.InverseView = glm::inverse(m_Camera.GetViewMatrix());
		m_SceneUB.CameraFrustum = CameraFrustum(m_Camera);
		m_SceneUB.CameraPosition = glm::vec4(m_Camera.GetPosition(), 0.0f);
		
		
		const uint32_t currentFrame = Renderer::GetAPIContext()->GetCurrentFrame();
		m_SceneBufferSet->Get(0, 0, currentFrame)->Update(&m_SceneUB, sizeof(UBScene), 0);
		
		if (m_NumUpdates)
		{
			m_StorageBufferSet->Update(m_SceneVoxels.data(), m_SceneVoxels.size() * sizeof(Voxel), 0, 3);
			m_NumUpdates--;
		}
		Renderer::BeginRenderPass(m_CommandBuffer, m_RenderPass, true);

		// Scene pipeline

		Renderer::BindPipeline(m_CommandBuffer, m_Pipeline, m_SceneBufferSet, m_StorageBufferSet, m_Material);
		Renderer::SubmitFullscreenQuad(m_CommandBuffer, m_Pipeline, m_MaterialInstance);
		
		// Character pipeline

		//Renderer::BindPipeline(m_CommandBuffer, m_CharacterPipeline, m_SceneBufferSet, m_CharacterBufferSet, m_CharacterMaterial);
		//Renderer::SubmitFullscreenQuad(m_CommandBuffer, m_CharacterPipeline, m_MaterialInstance);
		
		Renderer::EndRenderPass(m_CommandBuffer);

		m_CommandBuffer->EndTimestampQuery(s_TimeQueries.GPUTime);
		m_CommandBuffer->End();
		m_CommandBuffer->Submit();
	}

	void GameLayer::OnEvent(Event& event)
	{
		m_Camera.OnEvent(event);
		if (event.GetEventType() == EventType::MouseButtonPressed)
			onMouseButtonPress((MouseButtonPressEvent&)event);
	}

	void GameLayer::OnImGuiRender()
	{
		if (ImGui::Begin("Performance"))
		{
			const uint32_t frameIndex = Renderer::GetCurrentFrame();
			ImGui::Text("GPU Time: %f", m_CommandBuffer->GetExecutionGPUTime(frameIndex, s_TimeQueries.GPUTime));
		}
		ImGui::End();

		if (ImGui::Begin("Ray March"))
		{
			if (ImGui::DragFloat3("Light Direction", glm::value_ptr(m_SceneUB.LightDirection), 0.1f))
			{

			}
			if (ImGui::DragFloat3("Light Color", glm::value_ptr(m_SceneUB.LightColor), 0.1f))
			{

			}
			if (ImGui::DragFloat3("Chunk Position", glm::value_ptr(m_SceneUB.ChunkPosition), 0.1f))
			{

			}
			if (ImGui::DragInt("Max Traverse", (int*)&m_SceneUB.MaxTraverse))
			{
				
			}
			if (ImGui::DragFloat("Voxel Size", &m_SceneUB.VoxelSize, 0.1f))
			{

			}
			if (ImGui::ColorEdit4("Voxel Color", glm::value_ptr(m_Color)))
			{

			}
		
			if (ImGui::DragInt("Seed", &m_Seed))
			{
				generateVoxels();
			}
			if (ImGui::DragInt("Octaves", &m_Octaves))
			{
				generateVoxels();
			}
			if (ImGui::DragFloat("Frequency", &m_Frequency, 0.05f))
			{
				generateVoxels();
			}
	
			float fov = m_Camera.GetFOV();
			if (ImGui::DragFloat("FOV", &fov))
			{
				m_Camera.SetFOV(fov);
			}
			const float aspect = m_Camera.GetAspectRatio();
			ImGui::Text("Aspect %f", aspect);
			ImGui::Text("Camera Position %f %f %f", m_Camera.GetPosition().x, m_Camera.GetPosition().y, m_Camera.GetPosition().z);
			if (m_Generating)
				ImGui::Text("Generating Voxels!!!");
		}
		ImGui::End();
		displayStats();
	}

	bool GameLayer::onMouseButtonPress(MouseButtonPressEvent& event)
	{
		if (!Input::IsKeyPressed(KeyCode::KEY_LEFT_CONTROL))
		{
			if (event.IsButtonPressed(MouseCode::MOUSE_BUTTON_LEFT))
			{
				auto [mx, my] = GetMouseViewportSpace();
				auto [orig, dir] = castRay(mx, my);

				HitResult result;
				if (rayMarch(orig, dir, result))
				{
					m_SceneVoxels[result.HitVoxelIndex].Color = 0;
					return true;
				}
			}
			else if (event.IsButtonPressed(MouseCode::MOUSE_BUTTON_RIGHT))
			{
				if (m_LastPreviewVoxel < m_SceneVoxels.size())
				{
					m_SceneVoxels[m_LastPreviewVoxel].Color = ColorToUINT(m_Color);
					m_LastPreviewVoxel = m_SceneVoxels.size();
				}
			}
		}
		return false;
	}
	bool GameLayer::onMouseButtonRelease(MouseButtonReleaseEvent& event)
	{
		return false;
	}
	bool GameLayer::onWindowResize(WindowResizeEvent& event)
	{
		return false;
	}

	bool GameLayer::onKeyPress(KeyPressedEvent& event)
	{
		return false;
	}

	bool GameLayer::onKeyRelease(KeyReleasedEvent& event)
	{
		return false;
	}

	void GameLayer::generateVoxels()
	{

		if (m_Generating)
		{
			m_QueuedGenerate = true;
			return;
		}

		m_QueuedGenerate = false;
		m_Generating = true;
		Application::Get().GetThreadPool().PushJob([this]() {

			siv::PerlinNoise m_Noise{ (uint32_t) m_Seed};
			
			const double fx = (m_Frequency / SCENE_VOXEL_GRID_SIZE);
			const double fy = (m_Frequency / SCENE_VOXEL_GRID_SIZE);
			for (int i = 0; i < SCENE_VOXEL_GRID_SIZE; ++i)
			{
				for (int j = 0; j < SCENE_VOXEL_GRID_SIZE; ++j)
				{
					double x = i;
					double y = j;
					double val = m_Noise.octave2D(x * fx, y * fy, m_Octaves);
					int height = val * 16;
					for (int k = -16; k < height; ++k)
					{
						m_SceneVoxels[Index3D(i, k + 16, j, SCENE_VOXEL_GRID_SIZE, SCENE_VOXEL_GRID_SIZE)].Color = RandomColor();
					}
				}
			}
			m_Generating = false;
			m_NumUpdates = Renderer::GetConfiguration().FramesInFlight;
		});	
	}

	void GameLayer::setupScene()
	{
		std::vector<BufferLayout> layouts = { {
			   { ShaderDataType::Float3, "a_Position"},
			   { ShaderDataType::Float2, "a_TexCoord"}
		   } };

		m_Material = Material::Create(Shader::Create("RaymarchShader", "Assets/RaymarchShader_Vertex.glsl", "Assets/RaymarchShader_Frag.glsl", layouts));
		m_MaterialInstance = Ref<MaterialInstance>::Create(m_Material);
		m_Pipeline = CreatePipeline(m_Material->GetShader(), m_RenderPass);
		m_StorageBufferSet = StorageBufferSet::Create(3);

		m_SceneVoxels.resize(SCENE_VOXEL_GRID_SIZE * SCENE_VOXEL_GRID_SIZE * SCENE_VOXEL_GRID_SIZE);

		m_StorageBufferSet->Create(m_SceneVoxels.size() * sizeof(Voxel), 0, 3);

		for (int i = 0; i < SCENE_VOXEL_GRID_SIZE; ++i)
		{
			for (int j = 0; j < SCENE_VOXEL_GRID_SIZE; ++j)
			{
				for (int k = 0; k < SCENE_VOXEL_GRID_SIZE; ++k)
				{
					m_SceneVoxels[Index3D(i, j, k, SCENE_VOXEL_GRID_SIZE, SCENE_VOXEL_GRID_SIZE)].Color = 0;
				}
			}
		}

		generateVoxels();
		m_StorageBufferSet->Update(m_SceneVoxels.data(), m_SceneVoxels.size() * sizeof(Voxel), 0, 3);
	}

	void GameLayer::setupCharacter()
	{
		std::vector<BufferLayout> layouts = { {
			   { ShaderDataType::Float3, "a_Position"},
			   { ShaderDataType::Float2, "a_TexCoord"}
		   } };

		m_CharacterMaterial = Material::Create(Shader::Create("RaymarchShader", "Assets/RaymarchShader_Vertex.glsl", "Assets/RaymarchShader_Frag.glsl", layouts));
		m_CharacterMaterialInstance = Ref<MaterialInstance>::Create(m_CharacterMaterial);
		m_CharacterPipeline = CreatePipeline(m_CharacterMaterial->GetShader(), m_RenderPass);
		m_CharacterBufferSet = StorageBufferSet::Create(3);


		m_CharacterVoxels.resize(SCENE_VOXEL_GRID_SIZE * SCENE_VOXEL_GRID_SIZE * SCENE_VOXEL_GRID_SIZE);
		memset(m_CharacterVoxels.data(), 0, m_CharacterVoxels.size() * sizeof(Voxel));

		m_CharacterBufferSet->Create(m_CharacterVoxels.size() * sizeof(Voxel), 0, 3);

		std::ifstream output("chr_knight.vox", std::ios::binary);
		std::vector<uint8_t> data(std::istreambuf_iterator<char>(output), {});
		auto scene = ogt_vox_read_scene(data.data(), data.size());

		for (uint32_t i = 0; i < scene->num_models; ++i)
		{
			for (uint32_t x = 0; x < scene->models[i]->size_x; ++x)
			{
				for (uint32_t y = 0; y < scene->models[i]->size_y; ++y)
				{
					for (uint32_t z = 0; z < scene->models[i]->size_z; ++z)
					{
						const uint32_t index = Index3D(x, z, y, SCENE_VOXEL_GRID_SIZE, SCENE_VOXEL_GRID_SIZE);
						const uint32_t modelIndex = Index3D(x, y, z, scene->models[i]->size_x, scene->models[i]->size_y);
						const uint8_t colorIndex = scene->models[i]->voxel_data[modelIndex];
						const auto color = scene->palette.color[colorIndex];

						if (index < m_CharacterVoxels.size())
							m_CharacterVoxels[index].Color = ColorToUINT(color.r, color.g, color.b, color.a);
					}
				}
			}
		}
		m_CharacterBufferSet->Update(m_CharacterVoxels.data(), m_CharacterVoxels.size() * sizeof(Voxel), 0, 3);
	}

	void GameLayer::displayStats()
	{
		if (ImGui::Begin("Stats"))
		{
			const auto& stats = Renderer::GetStats();
			ImGui::Text("Draw Arrays: %d", stats.DrawArraysCount);
			ImGui::Text("Draw Indexed: %d", stats.DrawIndexedCount);
			ImGui::Text("Draw Instanced: %d", stats.DrawInstancedCount);
			ImGui::Text("Draw Fullscreen: %d", stats.DrawFullscreenCount);
			ImGui::Text("Draw Indirect: %d", stats.DrawIndirectCount);
			ImGui::Text("Commands Count: %d", stats.CommandsCount);
			ImGui::Text("GPU Time: %f", m_CommandBuffer->GetExecutionGPUTime(Renderer::GetCurrentFrame(), s_TimeQueries.GPUTime));
		}
		ImGui::End();
	}
	void GameLayer::voxelPreview()
	{
		auto [mx, my] = GetMouseViewportSpace();
		auto [orig, dir] = castRay(mx, my);
		HitResult result;
		if (rayMarch(orig, dir, result))
		{
			if (result.HitVoxelIndex == m_LastPreviewVoxel)
				return;

			if (m_LastPreviewVoxel < m_SceneVoxels.size())
			{
				m_SceneVoxels[m_LastPreviewVoxel].Color = 0;
			}
			m_LastPreviewVoxel = result.HitVoxelIndex;

			if (result.HitSide == HitResult::Top)
			{
				if (result.Y + 1 < SCENE_VOXEL_GRID_SIZE)
					m_LastPreviewVoxel = Index3D(result.X, result.Y + 1, result.Z, SCENE_VOXEL_GRID_SIZE, SCENE_VOXEL_GRID_SIZE);
			}
			else if (result.HitSide == HitResult::Bottom)
			{
				if ((int)result.Y - 1 >= 0)
					m_LastPreviewVoxel = Index3D(result.X, result.Y - 1, result.Z, SCENE_VOXEL_GRID_SIZE, SCENE_VOXEL_GRID_SIZE);
			}
			else if (result.HitSide == HitResult::Right)
			{
				if ((int)result.X - 1 >= 0)
					m_LastPreviewVoxel = Index3D(result.X - 1, result.Y, result.Z, SCENE_VOXEL_GRID_SIZE, SCENE_VOXEL_GRID_SIZE);
			}
			else if (result.HitSide == HitResult::Left)
			{
				if (result.X + 1 < SCENE_VOXEL_GRID_SIZE)
					m_LastPreviewVoxel = Index3D(result.X + 1, result.Y, result.Z, SCENE_VOXEL_GRID_SIZE, SCENE_VOXEL_GRID_SIZE);
			}
			else if (result.HitSide == HitResult::Back)
			{
				if ((int)result.Z - 1 >= 0)
					m_LastPreviewVoxel = Index3D(result.X, result.Y, result.Z - 1, SCENE_VOXEL_GRID_SIZE, SCENE_VOXEL_GRID_SIZE);
			}
			else if (result.HitSide == HitResult::Front)
			{
				if (result.Z + 1 < SCENE_VOXEL_GRID_SIZE)
					m_LastPreviewVoxel = Index3D(result.X, result.Y, result.Z + 1, SCENE_VOXEL_GRID_SIZE, SCENE_VOXEL_GRID_SIZE);
			}

			m_SceneVoxels[m_LastPreviewVoxel].Color = ColorToUINT(m_Color);
		}
	}
	bool GameLayer::rayMarch(const glm::vec3& rayOrig, const glm::vec3& rayDir, HitResult& result)
	{
		glm::vec3 origin = rayOrig + glm::vec3(m_SceneUB.ChunkPosition);
		glm::vec3 direction = rayDir;
		glm::ivec3 current_voxel = glm::ivec3(floor(origin / m_SceneUB.VoxelSize));

		glm::ivec3 step = glm::ivec3(
			(direction.x > 0.0) ? 1 : -1,
			(direction.y > 0.0) ? 1 : -1,
			(direction.z > 0.0) ? 1 : -1
		);
		glm::vec3 next_boundary = glm::vec3(
			float((step.x > 0) ? current_voxel.x + 1 : current_voxel.x) * m_SceneUB.VoxelSize,
			float((step.y > 0) ? current_voxel.y + 1 : current_voxel.y) * m_SceneUB.VoxelSize,
			float((step.z > 0) ? current_voxel.z + 1 : current_voxel.z) * m_SceneUB.VoxelSize
		);

		glm::vec3 t_max = (next_boundary - origin) / direction; // we will move along the axis with the smallest value
		glm::vec3 t_delta = m_SceneUB.VoxelSize / direction * glm::vec3(step);
		uint32_t voxel = 0;
		uint32_t i = 0;
		do
		{
			if (t_max.x < t_max.y && t_max.x < t_max.z)
			{
				t_max.x += t_delta.x;
				current_voxel.x += step.x;
				result.HitSide = (step.x < 0) ? HitResult::Left : HitResult::Right;
			}
			else if (t_max.y < t_max.z)
			{
				t_max.y += t_delta.y;
				current_voxel.y += step.y;
				result.HitSide = (step.y < 0) ? HitResult::Top : HitResult::Bottom;
			}
			else
			{
				t_max.z += t_delta.z;
				current_voxel.z += step.z;
				result.HitSide = (step.x < 0) ? HitResult::Front : HitResult::Back;
			}


			if (((  current_voxel.x < float(m_SceneUB.Width)  && current_voxel.x > 0)
				&& (current_voxel.y < float(m_SceneUB.Height) && current_voxel.y > 0)
				&& (current_voxel.z < float(m_SceneUB.Depth)  && current_voxel.z > 0)))
			{
				result.HitVoxelIndex = Index3D(current_voxel.x, current_voxel.y, current_voxel.z, m_SceneUB.Width, m_SceneUB.Height);
				result.X = current_voxel.x;
				result.Y = current_voxel.y;
				result.Z = current_voxel.z;
				voxel = m_SceneVoxels[result.HitVoxelIndex].Color;
				if (voxel != 0)
				{
					return true;
				}
			}
			i += 1;
		} while (voxel == 0 && i < m_SceneUB.MaxTraverse);

		return false;
	}
	std::pair<glm::vec3, glm::vec3> GameLayer::castRay(float mx, float my) const
	{
		const glm::vec4 mouseClipPos = { mx, my, -1.0f, 1.0f };

		const auto inverseProj = glm::inverse(m_Camera.GetProjectionMatrix());
		const auto inverseView = glm::inverse(glm::mat3(m_Camera.GetViewMatrix()));

		const glm::vec4 ray = inverseProj * mouseClipPos;
		glm::vec3 rayPos = m_Camera.GetPosition();
		glm::vec3 rayDir = inverseView * glm::vec3(ray);

		return { rayPos, glm::normalize(rayDir) };
	}
}