#include "stdafx.h"
#include "ParticleEffect.h"


namespace XYZ {

	namespace Helper {

	
		static std::string GetStatement(const char* str, const char** outPosition)
		{
			const char* end = strstr(str, ";");
			if (!end)
				return str;

			if (outPosition)
				*outPosition = end;
			size_t length = end - str + 1;
			return std::string(str, length);
		}

		static std::string GetEndStruct(const char* str, const char** outPosition)
		{
			const char* end = strstr(str, "};");
			if (!end)
				return str;

			if (outPosition)
				*outPosition = end;
			size_t length = end - str + 1;
			return std::string(str, length);
		}

		enum class ComputeShaderDataType
		{
			None, Int, UInt, Float, Float2, Float3, Float4
		};

		static ComputeShaderDataType StringToShaderDataType(const std::string& str)
		{
			if (str == "int")
				return ComputeShaderDataType::Int;
			if (str == "uint")
				return ComputeShaderDataType::UInt;
			if (str == "float")
				return ComputeShaderDataType::Float;
			if (str == "vec2")
				return ComputeShaderDataType::Float2;
			if (str == "vec3")
				return ComputeShaderDataType::Float3;
			if (str == "vec4")
				return ComputeShaderDataType::Float4;

			XYZ_ASSERT(false, "Invalid type");
			return ComputeShaderDataType::None;
		}

		struct ShaderVariable
		{
			std::string           Name;
			ComputeShaderDataType Type;

			size_t Size() const
			{
				switch (Type)
				{
				case XYZ::Helper::ComputeShaderDataType::None:
					return 0;
					break;
				case XYZ::Helper::ComputeShaderDataType::Int:
					return 4;
					break;
				case XYZ::Helper::ComputeShaderDataType::UInt:
					return 4;
					break;
				case XYZ::Helper::ComputeShaderDataType::Float:
					return 4;
					break;
				case XYZ::Helper::ComputeShaderDataType::Float2:
					return 2 * 4;
					break;
				case XYZ::Helper::ComputeShaderDataType::Float3:
					return 3 * 4;
					break;
				case XYZ::Helper::ComputeShaderDataType::Float4:
					return 4 * 4;
					break;
				}
				return 0;
			}
		};
		struct ShaderStruct
		{
			std::string Name;
			std::vector<ShaderVariable> Variables;
			size_t Size() const
			{
				size_t size = 0;
				for (auto& variable : Variables)
					size += variable.Size();
				return size;
			}
		};
		static ShaderStruct ParseStruct(const std::string& structSource)
		{
			ShaderStruct shaderStruct;
			std::vector<std::string> tokens = std::move(Utils::SplitString(structSource, "\t\n"));
			std::vector<std::string> structName = std::move(Utils::SplitString(tokens[0], " \r"));
			shaderStruct.Name = structName[1];
			for (size_t i = 1; i < tokens.size(); ++i)
			{
				std::vector<std::string> variables = std::move(Utils::SplitString(tokens[i], " \r"));
				if (variables.size() > 1)
				{
					ComputeShaderDataType type = StringToShaderDataType(variables[0]);
					variables[1].pop_back(); // pop ;
					std::string name = variables[1];
					shaderStruct.Variables.push_back({name, type});
				}
			}
			return shaderStruct;
		}

		static std::vector<ShaderStruct> ParseStructs(const std::string& source)
		{
			std::vector<ShaderStruct> structs;
			const char* token = nullptr;
			const char* src = source.c_str();
			while (token = Utils::FindToken(src, "struct"))
			{
				structs.push_back(ParseStruct(GetEndStruct(token, &src)));
			}			
			return structs;
		}

		struct ShaderBuffer
		{
			std::string					Name;
			uint32_t					Size;
			uint32_t					Binding;
			bool						Indirect;
			bool						RenderBuffer;
			bool						ParticleBuffer;
			std::vector<ShaderVariable> Variables; // This is used only if RenderBuffer is true
		};

		static bool ParseShaderBuffer(ShaderBuffer& buffer, const std::string& bufferSource, const std::vector<ShaderStruct>& structs)
		{
			std::vector<std::string> split = Utils::SplitString(bufferSource, " ,=}{\n\r\t");
			auto bindingIt = std::find(split.begin(), split.end(), "binding");
			if (bindingIt != split.end())
				buffer.Binding = atoi((++bindingIt)->c_str());
			
			auto bufferIt = std::find(bindingIt, split.end(), "buffer");
			if (bufferIt == split.end())
				return false;

			auto nameIt = (++bufferIt);
			buffer.Name = *nameIt;

			auto indirectIt = std::find(nameIt, split.end(), "indirect");
			buffer.Indirect = (indirectIt != split.end());

			auto renderIt = std::find(nameIt, split.end(), "render");
			buffer.RenderBuffer = (renderIt != split.end());

			auto particleIt = std::find(nameIt, split.end(), "particle");
			buffer.ParticleBuffer = (particleIt != split.end());

			for (auto& str : structs)
			{
				auto structIt = std::find(nameIt, split.end(), str.Name);
				if (structIt != split.end())
				{
					buffer.Size = (uint32_t)str.Size();
					if (buffer.RenderBuffer)
						buffer.Variables = str.Variables;
					break;
				}
			}
			return true;
		}

		static std::vector<ShaderBuffer> ParseShaderBuffers(const std::string& source, const std::vector<ShaderStruct>& structs)
		{
			std::vector<ShaderBuffer> buffers;
			size_t bufferStart = 0;
			size_t bufferEnd = 0;
			bufferStart = source.find("layout", bufferStart);
			bufferEnd = source.find("};", bufferStart);

			while (bufferStart != std::string::npos)
			{
				std::string bufferStr = source.substr(bufferStart, bufferEnd - bufferStart);
				ShaderBuffer buffer;
				if (ParseShaderBuffer(buffer, bufferStr, structs))
					buffers.push_back(buffer);
				bufferStart = source.find("layout", bufferStart + 1);
				bufferEnd = source.find("};", bufferStart);
			}
			return buffers;
		}

		struct ShaderCounter
		{
			std::string Name;
			uint32_t    Binding;
			uint32_t    Offset;
		};

		static bool ParseShaderCounter(ShaderCounter& counter, const std::string& counterSource)
		{
			std::vector<std::string> split = Utils::SplitString(counterSource, " ,=()\n\r\t");
			auto bindingIt = std::find(split.begin(), split.end(), "binding");
			if (bindingIt != split.end())
				counter.Binding = atoi((++bindingIt)->c_str());

			auto offsetIt = std::find(bindingIt, split.end(), "offset");
			if (offsetIt != split.end())
				counter.Offset = atoi(offsetIt->c_str());

			auto atomicIt = std::find(offsetIt, split.end(), "atomic_uint");
			if (atomicIt == split.end())
				return false;

			auto nameIt = (++atomicIt);
			counter.Name = *nameIt;
			
			return true;
		}

		static std::vector<ShaderCounter> ParseShaderCounters(const std::string& source)
		{
			std::vector<ShaderCounter> counters;
			size_t bufferStart = 0;
			size_t bufferEnd = 0;
			bufferStart = source.find("layout", bufferStart);
			bufferEnd = source.find("};", bufferStart);

			while (bufferStart != std::string::npos)
			{
				std::string bufferStr = source.substr(bufferStart, bufferEnd - bufferStart);
				ShaderCounter counter;
				if (ParseShaderCounter(counter, bufferStr))
					counters.push_back(counter);
				bufferStart = source.find("layout", bufferStart + 1);
				bufferEnd = source.find(";", bufferStart);
			}
			return counters;
		}
		static BufferLayout BufferLayoutFromVariables(const std::vector<ShaderVariable>& variables, uint32_t offset)
		{
			uint32_t counter = offset;
			std::vector<BufferElement> elements;
			for (auto& variable : variables)
			{
				switch (variable.Type)
				{
				case XYZ::Helper::ComputeShaderDataType::None:
					break;
				case XYZ::Helper::ComputeShaderDataType::Int:
					elements.emplace_back(counter++, ShaderDataComponent::Int, variable.Name, 1);
					break;
				case XYZ::Helper::ComputeShaderDataType::UInt:
					elements.emplace_back(counter++, ShaderDataComponent::Int, variable.Name, 1);
					break;
				case XYZ::Helper::ComputeShaderDataType::Float:
					elements.emplace_back(counter++, ShaderDataComponent::Float, variable.Name, 1);
					break;
				case XYZ::Helper::ComputeShaderDataType::Float2:
					elements.emplace_back(counter++, ShaderDataComponent::Float2, variable.Name, 1);
					break;
				case XYZ::Helper::ComputeShaderDataType::Float3:
					elements.emplace_back(counter++, ShaderDataComponent::Float3, variable.Name, 1);
					break;
				case XYZ::Helper::ComputeShaderDataType::Float4:
					elements.emplace_back(counter++, ShaderDataComponent::Float4, variable.Name, 1);
					break;
				}
			}
			return BufferLayout(elements);
		}

	}
	ParticleEffect::ParticleEffect(uint32_t maxParticles, const Ref<Shader>& computeShader)
		:
		m_ComputeShader(computeShader),
		m_VertexArray(VertexArray::Create()),
		m_MaxParticles(maxParticles),
		m_Rate(0.0f),
		m_EmittedParticles(0.0f),
		m_PlayTime(0.0f)
	{
		ParticleVertex quad[4] = {
			ParticleVertex{glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec2(0.0f, 0.0f)},
			ParticleVertex{glm::vec3(0.5f, -0.5f, 0.0f),  glm::vec2(1.0f, 0.0f)},
			ParticleVertex{glm::vec3(0.5f,  0.5f, 0.0f),  glm::vec2(1.0f, 1.0f)},
			ParticleVertex{glm::vec3(-0.5f,  0.5f, 0.0f), glm::vec2(0.0f, 1.0f)}
		};
		Ref<VertexBuffer> squareVBpar;
		squareVBpar = XYZ::VertexBuffer::Create(quad, 4 * sizeof(ParticleVertex));
		squareVBpar->SetLayout({
			{ 0, XYZ::ShaderDataComponent::Float3, "a_Position" },
			{ 1, XYZ::ShaderDataComponent::Float2, "a_TexCoord" }
			});
		m_VertexArray->AddVertexBuffer(squareVBpar);

		std::ifstream in(computeShader->GetPath(), std::ios::in | std::ios::binary);
		if (in)
		{
			std::string source;
			in.seekg(0, std::ios::end);
			source.resize(in.tellg());
			in.seekg(0, std::ios::beg);
			in.read(&source[0], source.size());
			in.close();
			std::vector<Helper::ShaderStruct>  structs = std::move(Helper::ParseStructs(source));
			std::vector<Helper::ShaderBuffer>  buffers = std::move(Helper::ParseShaderBuffers(source, structs));
			std::vector<Helper::ShaderCounter> counters = std::move(Helper::ParseShaderCounters(source));
			
			for (auto& buffer : buffers)
			{
				if (buffer.Indirect)
				{
					XYZ_ASSERT(!m_DrawCommand.Raw(), "Only one indirect buffer is supported");
					m_DrawCommand = IndirectBuffer::Create(nullptr, buffer.Size, buffer.Binding);
				}
				else
				{
					Buffer buf;
					buf.Name = buffer.Name;
					buf.Storage = ShaderStorageBuffer::Create(buffer.Size, buffer.Binding);
					buf.ElementSize = buffer.Size;
					buf.ElementCount = 0;
					buf.RenderBuffer = buffer.RenderBuffer;
					buf.ParticleBuffer = buffer.ParticleBuffer;

					m_Buffers.push_back(buf);
					if (buffer.RenderBuffer)
					{
						buf.Storage->SetLayout(Helper::BufferLayoutFromVariables(buffer.Variables, 2));
						m_VertexArray->AddShaderStorageBuffer(buf.Storage);
					}
				}
			}
			for (auto& counter : counters)
			{
				Counter cou;
				cou.Name = counter.Name;
				cou.Atomic = AtomicCounter::Create(1, counter.Binding);
				m_Counters.push_back(cou);
			}
		}

		uint32_t squareIndpar[] = { 0, 1, 2, 2, 3, 0 };
		Ref<XYZ::IndexBuffer> squareIBpar;
		squareIBpar = XYZ::IndexBuffer::Create(squareIndpar, sizeof(squareIndpar) / sizeof(uint32_t));
		m_VertexArray->SetIndexBuffer(squareIBpar);
	}
	void ParticleEffect::Update(Timestep ts)
	{
		float raise = m_Rate * ts;
		if (m_EmittedParticles + raise <= m_MaxParticles)
			m_EmittedParticles += raise;

		uint32_t emitted = (uint32_t)std::ceil(m_EmittedParticles);
		for (auto& buffer : m_Buffers)
		{
			if (buffer.ParticleBuffer)
				buffer.ElementCount = emitted;
		}
		m_PlayTime += ts;
	}

	void ParticleEffect::Compute()
	{
		for (auto& buffer : m_Buffers)
		{
			buffer.Storage->BindRange(0, buffer.ElementCount * buffer.ElementSize);
		}
		m_ComputeShader->Compute(32, 32, 1);
	}

	void ParticleEffect::Reset()
	{
		for (auto& counter : m_Counters)
		{
			counter.Atomic->Reset();
		}
		m_EmittedParticles = 0.0f;
		m_PlayTime = 0.0f;
	}

	void ParticleEffect::SetBufferSize(const std::string& name, uint32_t size)
	{
		for (auto& buffer : m_Buffers)
		{
			if (buffer.Name == name)
			{
				buffer.Storage->Resize(nullptr, size);
				return;
			}
		}
		XYZ_ASSERT(false, "No buffer with the name ", name);
	}
	void ParticleEffect::SetBufferData(const std::string& name, void* data, uint32_t count, uint32_t elementSize, uint32_t offset)
	{
		for (auto& buffer : m_Buffers)
		{
			if (buffer.Name == name)
			{
				XYZ_ASSERT(elementSize == buffer.ElementSize, "Wrong element size");
				buffer.Storage->Update(data, count * elementSize, offset);
				return;
			}
		}
		XYZ_ASSERT(false, "No buffer with the name ", name);
	}
}