#include "stdafx.h"
#include "ParticleDataBuffer.h"


namespace XYZ {


	ParticleDataBuffer::ParticleDataBuffer(uint32_t maxParticles)
		:
		m_MaxParticles(maxParticles),
		m_AliveParticles(0)
	{
		if (maxParticles)
		{
			generateParticles(maxParticles);
		}
		else
		{
			Particle	= nullptr;
			TexOffset	= nullptr;
			StartColor	= nullptr;
			EndColor	= nullptr;
			Size		= nullptr;
			Rotation    = nullptr;
		}
	}
	ParticleDataBuffer::ParticleDataBuffer(ParticleDataBuffer&& other) noexcept
	{
		Particle = other.Particle;
		TexOffset = other.TexOffset;
		StartColor = other.StartColor;
		EndColor = other.EndColor;
		Size = other.Size;
		Rotation = other.Rotation;

		m_MaxParticles = other.m_MaxParticles;
		m_AliveParticles = other.m_AliveParticles;

		other.Particle = nullptr;
		other.TexOffset = nullptr;
		other.StartColor = nullptr;
		other.EndColor = nullptr;
		other.Size = nullptr;
		other.Rotation = nullptr;
		other.m_MaxParticles = 0;
		other.m_AliveParticles = 0;
	}
	ParticleDataBuffer::~ParticleDataBuffer()
	{
		deleteParticles();
	}

	ParticleDataBuffer::ParticleDataBuffer(const ParticleDataBuffer& other)
		:
		m_MaxParticles(other.m_MaxParticles),
		m_AliveParticles(other.m_AliveParticles)
	{
		if (m_MaxParticles)
		{
			generateParticles(m_MaxParticles);
			copyData(other);
		}
		else
		{
			Particle = nullptr;
			TexOffset = nullptr;
			StartColor = nullptr;
			EndColor = nullptr;
			Size = nullptr;
			Rotation = nullptr;
		}
	}
	
	ParticleDataBuffer& ParticleDataBuffer::operator=(const ParticleDataBuffer& other)
	{
		m_MaxParticles = other.m_MaxParticles;
		m_AliveParticles = other.m_AliveParticles;
		
		deleteParticles();
		if (m_MaxParticles)
		{
			generateParticles(m_MaxParticles);
			copyData(other);
		}
		else
		{
			Particle = nullptr;
			TexOffset = nullptr;
			StartColor = nullptr;
			EndColor = nullptr;
			Size = nullptr;
			Rotation = nullptr;
		}	
		return *this;
	}

	ParticleDataBuffer& ParticleDataBuffer::operator=(ParticleDataBuffer&& other) noexcept
	{
		deleteParticles();
		Particle = other.Particle;
		TexOffset = other.TexOffset;
		StartColor = other.StartColor;
		EndColor = other.EndColor;
		Size = other.Size;
		Rotation = other.Rotation;

		m_MaxParticles = other.m_MaxParticles;
		m_AliveParticles = other.m_AliveParticles;

		other.Particle = nullptr;
		other.TexOffset = nullptr;
		other.StartColor = nullptr;
		other.EndColor = nullptr;
		other.Size = nullptr;
		other.Rotation = nullptr;
		other.m_MaxParticles = 0;
		other.m_AliveParticles = 0;
		return *this;
	}
	void ParticleDataBuffer::SetMaxParticles(uint32_t maxParticles)
	{
		deleteParticles();
		m_MaxParticles = maxParticles;
		generateParticles(maxParticles);
		m_AliveParticles = 0;
	}
	void ParticleDataBuffer::Wake(uint32_t id)
	{
		Particle[id].Alive = true;
		m_AliveParticles++;
	}
	void ParticleDataBuffer::Kill(uint32_t id)
	{
		swapData(id, m_AliveParticles - 1);
		Particle[m_AliveParticles - 1].Alive = false;
		m_AliveParticles--;
	}
	void ParticleDataBuffer::generateParticles(uint32_t particleCount)
	{
		Particle		  = new ParticleData[particleCount];
		TexOffset		  = new glm::vec2[particleCount];
		StartColor        = new glm::vec4[particleCount];
		EndColor          = new glm::vec4[particleCount];
		Size			  = new glm::vec3[particleCount];
		Rotation		  = new glm::quat[particleCount];
		for (size_t i = 0; i < particleCount; ++i)
			Particle[i].Alive = false;
	}
	void ParticleDataBuffer::swapData(uint32_t a, uint32_t b)
	{
		Particle[a]			 = Particle[b];
		TexOffset[a]		 = TexOffset[b];
		StartColor[a]		 = StartColor[b];
		EndColor[a]			 = EndColor[b];
		Size[a]				 = Size[b];
		Rotation[a]			 = Rotation[b];
	}

	template <typename T>
	static void copy(T* dest, T* source, uint32_t count)
	{
		memcpy(dest, source, count * sizeof(T));
	}

	void ParticleDataBuffer::copyData(const ParticleDataBuffer& source)
	{
		copy(Particle,   source.Particle,	m_MaxParticles);
		copy(TexOffset,  source.TexOffset,	m_MaxParticles);
		copy(StartColor, source.StartColor,	m_MaxParticles);
		copy(EndColor,   source.EndColor,	m_MaxParticles);
		copy(Size,	     source.Size,		m_MaxParticles);
		copy(Rotation,   source.Rotation,	m_MaxParticles);	
	}

	void ParticleDataBuffer::deleteParticles()
	{
		delete[]Particle;
		delete[]TexOffset;
		delete[]StartColor;
		delete[]EndColor;
		delete[]Size;
		delete[]Rotation;
	}
}