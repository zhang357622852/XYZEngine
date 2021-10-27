#include "stdafx.h"
#include "ParticleSystemCPU.h"
#include "XYZ/Renderer/SceneRenderer.h"

#include "XYZ/Scene/Components.h"
#include "XYZ/Core/Application.h"

#include "XYZ/Debug/Profiler.h"

namespace XYZ {
	ParticleSystemCPU::ParticleSystemCPU()
		:
		m_ParticleData(0),
		m_MaxParticles(0),
		m_Play(false)
	{
	}
	ParticleSystemCPU::ParticleSystemCPU(uint32_t maxParticles)
		:
		m_ParticleData(maxParticles)
	{
		{
			ScopedLock<RenderData> write = m_RenderThreadPass.Write();
			write->m_RenderParticleData.resize(maxParticles);
		}
		{
			ScopedLock<RenderData> read = m_RenderThreadPass.Read();
			read->m_RenderParticleData.resize(maxParticles);
		}
	}

	ParticleSystemCPU::~ParticleSystemCPU()
	{
	}

	ParticleSystemCPU::ParticleSystemCPU(const ParticleSystemCPU& other)
		:
		m_ParticleData(other.m_ParticleData),
		m_UpdateThreadPass(other.m_UpdateThreadPass),
		m_EmitThreadPass(other.m_EmitThreadPass),
		m_RenderThreadPass(other.m_RenderThreadPass),
		m_LightPass(other.m_LightPass),
		m_MaxParticles(other.m_MaxParticles),
		m_Play(other.m_Play)
	{
	}

	ParticleSystemCPU::ParticleSystemCPU(ParticleSystemCPU&& other) noexcept
		:
		m_ParticleData(std::move(other.m_ParticleData)),
		m_UpdateThreadPass(std::move(other.m_UpdateThreadPass)),
		m_EmitThreadPass(std::move(other.m_EmitThreadPass)),
		m_RenderThreadPass(std::move(other.m_RenderThreadPass)),
		m_LightPass(std::move(other.m_LightPass)),
		m_MaxParticles(other.m_MaxParticles),
		m_Play(other.m_Play)
	{
	}

	ParticleSystemCPU& ParticleSystemCPU::operator=(const ParticleSystemCPU& other)
	{
		m_ParticleData = other.m_ParticleData;
		m_UpdateThreadPass = other.m_UpdateThreadPass;
		m_EmitThreadPass = other.m_EmitThreadPass;
		m_RenderThreadPass = other.m_RenderThreadPass;
		m_LightPass = other.m_LightPass;
		m_MaxParticles = other.m_MaxParticles;
		m_Play = other.m_Play;
		return *this;
	}

	ParticleSystemCPU& ParticleSystemCPU::operator=(ParticleSystemCPU&& other) noexcept
	{
		m_ParticleData = std::move(other.m_ParticleData);
		m_UpdateThreadPass = std::move(other.m_UpdateThreadPass);
		m_EmitThreadPass = std::move(other.m_EmitThreadPass);
		m_RenderThreadPass = std::move(other.m_RenderThreadPass);
		m_LightPass = std::move(other.m_LightPass);
		m_MaxParticles = other.m_MaxParticles;
		m_Play = other.m_Play;
		return *this;
	}

	void ParticleSystemCPU::Update(Timestep ts)
	{	
		XYZ_PROFILE_FUNC("ParticleSystemCPU::Update");
		if (m_Play)
		{		
			particleThreadUpdate(ts.GetSeconds());
		}
	}

	void ParticleSystemCPU::SubmitLights(Ref<SceneRenderer> renderer)
	{
		XYZ_PROFILE_FUNC("ParticleSystemCPU::SubmitLights");
		auto updateData = m_UpdateThreadPass.GetRead<UpdateData>();
		const auto& lightUpdater = updateData->m_LightUpdater;
		if (lightUpdater.m_TransformEntity.IsValid() 
		 && lightUpdater.m_LightEntity.IsValid()
		 && lightUpdater.m_LightEntity.HasComponent<PointLight2D>())
		{
			auto particleData = m_ParticleData.GetRead<ParticleDataBuffer>();
			if (lightUpdater.m_Enabled)
			{
				const PointLight2D& light = lightUpdater.m_LightEntity.GetComponent<PointLight2D>();
				const TransformComponent& transform = lightUpdater.m_TransformEntity.GetComponent<TransformComponent>();
				uint32_t count = std::min(lightUpdater.m_MaxLights, particleData->GetAliveParticles());
				for (uint32_t i = 0; i < count; ++i)
					renderer->SubmitLight(light, transform.WorldTransform * glm::translate(particleData->m_Lights[i]));
			}
		}
	}

	void ParticleSystemCPU::Play()
	{
		m_Play = true;
	}
	void ParticleSystemCPU::Stop()
	{
		m_Play = false;
	}

	void ParticleSystemCPU::SetMaxParticles(uint32_t maxParticles)
	{
		m_ParticleData.Get<ParticleDataBuffer>()->SetMaxParticles(maxParticles);
		{
			m_RenderThreadPass.Read()->m_RenderParticleData.resize(maxParticles);
		}
		m_RenderThreadPass.Swap();
		{
			m_RenderThreadPass.Read()->m_RenderParticleData.resize(maxParticles);
		}
	}

	uint32_t ParticleSystemCPU::GetMaxParticles() const
	{
		return m_MaxParticles;
	}


	ScopedLock<ParticleDataBuffer> ParticleSystemCPU::GetParticleData()
	{
		return m_ParticleData.Get<ParticleDataBuffer>();
	}

	ScopedLockRead<ParticleDataBuffer> ParticleSystemCPU::GetParticleDataRead() const
	{
		return m_ParticleData.GetRead<ParticleDataBuffer>();
	}


	ScopedLock<ParticleSystemCPU::UpdateData> ParticleSystemCPU::GetUpdateData()
	{
		return m_UpdateThreadPass.Get<UpdateData>();
	}

	ScopedLockRead<ParticleSystemCPU::UpdateData> ParticleSystemCPU::GetUpdateDataRead() const
	{
		return m_UpdateThreadPass.GetRead<UpdateData>();
	}

	ScopedLock<ParticleEmitterCPU> ParticleSystemCPU::GetEmitter()
	{
		return m_EmitThreadPass.Get<ParticleEmitterCPU>();
	}

	ScopedLockRead<ParticleEmitterCPU> ParticleSystemCPU::GetEmitterRead() const
	{
		return m_EmitThreadPass.GetRead<ParticleEmitterCPU>();
	}

	ScopedLock<ParticleSystemCPU::RenderData> ParticleSystemCPU::GetRenderData()
	{
		return m_RenderThreadPass.Read();
	}

	ScopedLockRead<ParticleSystemCPU::RenderData> ParticleSystemCPU::GetRenderDataRead() const
	{
		return m_RenderThreadPass.ReadRead();
	}

	void ParticleSystemCPU::particleThreadUpdate(float timestep)
	{
		Application::Get().GetThreadPool().PushJob<void>([this, timestep]() {
			XYZ_PROFILE_FUNC("ParticleSystemCPU::particleThreadUpdate Job");
			{
				ScopedLock<ParticleDataBuffer> particleData = m_ParticleData.Get<ParticleDataBuffer>();
				update(timestep, particleData.As());
				emit(timestep, particleData.As());
				buildRenderData(particleData.As());
			}
			m_RenderThreadPass.AttemptSwap();		
		});
	}
	void ParticleSystemCPU::update(Timestep timestep, ParticleDataBuffer& particles)
	{
		XYZ_PROFILE_FUNC("ParticleSystemCPU::update");
		ScopedLockRead<UpdateData> data = m_UpdateThreadPass.GetRead<UpdateData>();
		data->m_TimeUpdater.UpdateParticles(timestep, &particles);
		data->m_PositionUpdater.UpdateParticles(timestep, &particles);
		data->m_LightUpdater.UpdateParticles(timestep, &particles);
	}
	void ParticleSystemCPU::emit(Timestep timestep, ParticleDataBuffer& particles)
	{
		XYZ_PROFILE_FUNC("ParticleSystemCPU::emit");
		ScopedLock<ParticleEmitterCPU> data = m_EmitThreadPass.Get<ParticleEmitterCPU>();
		data->Emit(timestep, particles);
	}
	void ParticleSystemCPU::buildRenderData(ParticleDataBuffer& particles)
	{
		XYZ_PROFILE_FUNC("ParticleSystemCPU::buildRenderData");
		ScopedLock<RenderData> val = m_RenderThreadPass.Write();
		uint32_t endId = particles.GetAliveParticles();
		for (uint32_t i = 0; i < endId; ++i)
		{
			auto& particle = particles.m_Particle[i];
			val->m_RenderParticleData[i] = ParticleRenderData{
				particle.Color,
				particles.m_TexCoord[i],
				glm::vec2(particle.Position.x, particle.Position.y),
				particles.m_Size[i],
				particles.m_Rotation[i]
			};
		}
		val->m_InstanceCount = endId;
	}
	ParticleSystemCPU::RenderData::RenderData()
		:
		m_InstanceCount(0)
	{
	}
	ParticleSystemCPU::RenderData::RenderData(uint32_t maxParticles)
		:
		m_InstanceCount(0)
	{
		m_RenderParticleData.resize(maxParticles);
	}
}