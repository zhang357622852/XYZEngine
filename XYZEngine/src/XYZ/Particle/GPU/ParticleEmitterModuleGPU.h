#pragma once

#include "ParticleSystemLayout.h"

#include <glm/glm.hpp>

namespace XYZ {

	enum class ParticleEmitterModuleType
	{
		Spawn,
		BoxPosition,
		NumTypes
	};

	class XYZ_API ParticleEmitterModuleGPU : public RefCount
	{
	public:
		ParticleEmitterModuleGPU(ParticleEmitterModuleType type, uint32_t stride, std::vector<uint32_t> strideOffsets);

		virtual void Generate(std::byte* buffer, uint32_t count) const = 0;

		ParticleEmitterModuleType GetType() const { return m_Type; };

		bool Enabled;
	protected:
		ParticleEmitterModuleType m_Type;
		std::vector<uint32_t>	  m_StrideOffsets;
		uint32_t				  m_Stride;		  // Size of particle structure
	};


	class SpawnParticleEmitterModuleGPU : public ParticleEmitterModuleGPU
	{
	public:
		SpawnParticleEmitterModuleGPU(uint32_t stride, std::vector<uint32_t> strideOffsets);

		virtual void Generate(std::byte* buffer, uint32_t count) const override;


		float MinimumLife;
		float MaximumLife;
	};

	class TestParticleEmitterModuleGPU : public ParticleEmitterModuleGPU
	{
	public:
		TestParticleEmitterModuleGPU(uint32_t stride, std::vector<uint32_t> strideOffsets);

		virtual void Generate(std::byte* buffer, uint32_t count) const override;
	};

	class BoxParticleEmitterModuleGPU : public ParticleEmitterModuleGPU
	{
	public:
		BoxParticleEmitterModuleGPU(uint32_t stride, std::vector<uint32_t> strideOffsets);

		virtual void Generate(std::byte* buffer, uint32_t count) const override;

		glm::vec3 BoxMinimum;
		glm::vec3 BoxMaximum;
	};
}