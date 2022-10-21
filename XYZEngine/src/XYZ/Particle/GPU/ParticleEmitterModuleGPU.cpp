#include "stdafx.h"
#include "ParticleEmitterModuleGPU.h"

#include <glm/common.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/constants.hpp>

namespace XYZ {
	
	
	ParticleEmitterModuleGPU::ParticleEmitterModuleGPU(ParticleEmitterModuleType type, uint32_t stride, uint32_t strideOffset)
		:
		m_Type(type),
		m_Stride(stride),
		m_StrideOffset(strideOffset),
		Enabled(true)
	{
	}


	BoxParticleEmitterModuleGPU::BoxParticleEmitterModuleGPU(uint32_t stride, uint32_t strideOffset)
		:
		ParticleEmitterModuleGPU(ParticleEmitterModuleType::BoxPosition, stride, strideOffset),
		BoxMinimum(-1.0f),
		BoxMaximum(1.0f)
	{
	}
	void BoxParticleEmitterModuleGPU::Generate(std::byte* buffer, uint32_t count) const
	{
		uint32_t offset = m_StrideOffset;
		for (uint32_t i = 0; i < count; ++i)
		{
			glm::vec3* elem = reinterpret_cast<glm::vec3*>(&buffer[offset]);
			*elem = glm::linearRand(BoxMinimum, BoxMaximum);
			offset += m_Stride;
		}
	}
	SpawnParticleEmitterModuleGPU::SpawnParticleEmitterModuleGPU(uint32_t stride, uint32_t strideOffset)
		:
		ParticleEmitterModuleGPU(ParticleEmitterModuleType::Spawn, stride, strideOffset),
		MinimumLife(1.0f),
		MaximumLife(5.0f)
	{
	}
	void SpawnParticleEmitterModuleGPU::Generate(std::byte* buffer, uint32_t count) const
	{
		uint32_t offset = m_StrideOffset;
		for (uint32_t i = 0; i < count; ++i)
		{
			float* elem = reinterpret_cast<float*>(&buffer[offset]);
			*elem = glm::linearRand(MinimumLife, MaximumLife);
			offset += m_Stride;
		}
	}
	TestParticleEmitterModuleGPU::TestParticleEmitterModuleGPU(uint32_t stride, uint32_t strideOffset)
		:
		ParticleEmitterModuleGPU(ParticleEmitterModuleType::Spawn, stride, strideOffset)
	{
	}
	void TestParticleEmitterModuleGPU::Generate(std::byte* buffer, uint32_t count) const
	{
		uint32_t offset = m_StrideOffset;
		for (uint32_t i = 0; i < count; ++i)
		{
			uint32_t currOffset = offset;
			glm::vec4* startColor    = reinterpret_cast<glm::vec4*>(&buffer[currOffset]);
			*startColor = glm::linearRand(glm::vec4(0.5f, 0.5f, 0.5f, 1.0f), glm::vec4(1.0f));
			currOffset += sizeof(glm::vec4);

			glm::vec4* startRotation = reinterpret_cast<glm::vec4*>(&buffer[currOffset]);
			*startRotation = glm::linearRand(glm::vec4(-90.0f), glm::vec4(90.0f));
			currOffset += sizeof(glm::vec4);

			float startscale = glm::linearRand(0.1f, 2.0f);
			float endscale = glm::linearRand(0.1f, 2.0f);

			glm::vec4* startScale    = reinterpret_cast<glm::vec4*>(&buffer[currOffset]);
			*startScale = glm::vec4(startscale);
			currOffset += sizeof(glm::vec4);

			glm::vec4* startVelocity = reinterpret_cast<glm::vec4*>(&buffer[currOffset]);
			*startVelocity = glm::linearRand(glm::vec4(-8.0f), glm::vec4(8.0f));
			currOffset += sizeof(glm::vec4);

			glm::vec4* endColor      = reinterpret_cast<glm::vec4*>(&buffer[currOffset]);
			*endColor = glm::linearRand(glm::vec4(0.8f, 0.4f, 0.5f, 1.0f), glm::vec4(1.0f));
			currOffset += sizeof(glm::vec4);

			glm::vec4* endRotation   = reinterpret_cast<glm::vec4*>(&buffer[currOffset]);
			*endRotation = glm::linearRand(glm::vec4(-90.0f), glm::vec4(90.0f));
			currOffset += sizeof(glm::vec4);

			glm::vec4* endScale      = reinterpret_cast<glm::vec4*>(&buffer[currOffset]);
			*endScale = glm::vec4(endscale);
			currOffset += sizeof(glm::vec4);

			glm::vec4* endVelocity   = reinterpret_cast<glm::vec4*>(&buffer[currOffset]);
			*endVelocity = glm::linearRand(glm::vec4(-0.5f), glm::vec4(0.5f));
			currOffset += sizeof(glm::vec4);

			offset += m_Stride;
		}
	}
}