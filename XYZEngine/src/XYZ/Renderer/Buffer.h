#pragma once
#include "XYZ/Core/Ref/Ref.h"
#include "XYZ/Core/Assert.h"
#include "XYZ/Utils/DataStructures/ByteBuffer.h"

#include <memory>

namespace XYZ {

	enum class ShaderDataType
	{
		None = 0, 
		Float, 
		Float2, 
		Float3, 
		Float4, 
		Mat3, 
		Mat4, 
		Int, 
		Int2, 
		Int3, 
		Int4, 
		Bool
	};


	static unsigned int ShaderDataTypeSize(ShaderDataType type)
	{
		switch (type)
		{
		case ShaderDataType::Float:	return 4;
		case ShaderDataType::Float2:	return 4 * 2;
		case ShaderDataType::Float3:	return 4 * 3;
		case ShaderDataType::Float4:	return 4 * 4;
		case ShaderDataType::Mat3:		return 4 * 3 * 3;
		case ShaderDataType::Mat4:		return 4 * 4 * 4;
		case ShaderDataType::Int:		return 4;
		case ShaderDataType::Int2:		return 4 * 2;
		case ShaderDataType::Int3:		return 4 * 3;
		case ShaderDataType::Int4:		return 4 * 4;
		case ShaderDataType::Bool:		return 1;
		}
		XYZ_ASSERT(false, "Buffer: Unknown ShaderDataType");
		return 0;
	}

	struct BufferElement
	{
		BufferElement(ShaderDataType type, const std::string_view name)
			: Type(type), Size(ShaderDataTypeSize(type)), Offset(0)
		{}


		uint32_t GetComponentCount() const
		{
			switch (Type)
			{
			case ShaderDataType::Bool:   return 1;
			case ShaderDataType::Float:  return 1;
			case ShaderDataType::Float2: return 2;
			case ShaderDataType::Float3: return 3;
			case ShaderDataType::Float4: return 4;
			case ShaderDataType::Int:    return 1;
			case ShaderDataType::Int2:   return 2;
			case ShaderDataType::Int3:   return 3;
			case ShaderDataType::Int4:   return 4;
			case ShaderDataType::Mat3:   return 9;
			case ShaderDataType::Mat4:   return 16;
			}
			XYZ_ASSERT(false, "ShaderDataTypeSize(ShaderDataType::None)");
			return 0;
		}

		ShaderDataType Type;
		uint32_t	   Size;
		uint32_t	   Offset;
	};

	class BufferLayout
	{
	public:
		BufferLayout(bool instanced = false)
		: m_Instanced(instanced) {};

		BufferLayout(const std::initializer_list<BufferElement>& elements, bool instanced = false)
			: m_Elements(elements), m_Instanced(instanced)
		{
			createMat4();
			calculateOffsetsAndStride();
		}
		BufferLayout(const std::vector<BufferElement>& elements, bool instanced = false)
			: m_Elements(elements), m_Instanced(instanced)
		{
			createMat4();
			calculateOffsetsAndStride();
		}


		inline const uint32_t& GetStride() const { return m_Stride; }


		inline const std::vector<BufferElement>& GetElements() const { return m_Elements; }
		inline const bool Empty() const { return m_Elements.empty(); }
		inline const bool Instanced() const { return m_Instanced; }

		auto begin() { return m_Elements.begin(); }
		auto end() { return m_Elements.end(); }
		auto begin() const { return m_Elements.begin(); }
		auto end() const { return m_Elements.end(); }
	private:

		inline void calculateOffsetsAndStride()
		{
			uint32_t offset = 0;
			m_Stride = 0;
			for (auto& element : m_Elements)
			{
				element.Offset = offset;
				offset += element.Size;
				m_Stride += element.Size;
			}
		};


		inline void createMat4()
		{
			for (auto& element : m_Elements)
			{
				if (element.Type == ShaderDataType::Mat4)
				{
					element.Type = ShaderDataType::Float4;
					element.Size = 4 * 4;

					const BufferElement tmpElement = element;
					m_Elements.push_back(BufferElement(tmpElement.Type, ""));
					m_Elements.push_back(BufferElement(tmpElement.Type, ""));
					m_Elements.push_back(BufferElement(tmpElement.Type, ""));
				}
			}
		}

	private:
		std::vector<BufferElement> m_Elements;
		bool					   m_Instanced;
		uint32_t				   m_Stride = 0;

	};


	enum class BufferUsage
	{
		None = 0, Static = 1, Dynamic = 2
	};

	class VertexBuffer : public RefCount
	{
	public:
		virtual ~VertexBuffer() = default;
		virtual void Bind() const = 0;
		virtual void UnBind() const = 0;
		virtual void Update(const void* vertices, uint32_t size, uint32_t offset = 0) = 0;
		virtual void RT_Update(const void* vertices, uint32_t size, uint32_t offset = 0) {};
		virtual void Resize(const void* vertices, uint32_t size) = 0;
		virtual void SetLayout(const BufferLayout& layout) = 0;
		virtual const BufferLayout& GetLayout() const = 0;
		virtual uint32_t GetRendererID() const = 0;
		virtual uint32_t GetSize() const { return 0; }
		static Ref<VertexBuffer> Create(uint32_t size);

		static Ref<VertexBuffer> Create(const void* vertices, uint32_t size, BufferUsage usage = BufferUsage::Static);
	};


	enum class IndexType
	{
		Uint8, Uint16, Uint32
	};

	class IndexBuffer : public RefCount
	{
	public:
		virtual ~IndexBuffer() = default;
		virtual void Bind() const = 0;
		virtual void UnBind() const = 0;

		virtual uint32_t GetCount() const = 0;
		virtual uint32_t GetRendererID() const = 0;

		static Ref<IndexBuffer> Create(const void* indices, uint32_t count, IndexType type = IndexType::Uint32);
	};



	class StorageBuffer : public RefCount
	{
	public:
		virtual ~StorageBuffer() = default;
		virtual void BindBase(uint32_t binding) const {};
		virtual void BindRange(uint32_t offset, uint32_t size) const {};
		virtual void Bind() const {};

		virtual void Update(const void* data, uint32_t size, uint32_t offset = 0) {};
		virtual void RT_Update(const void* data, uint32_t size, uint32_t offset = 0) {};
		virtual void Update(ByteBuffer data, uint32_t size, uint32_t offset = 0) {}
		virtual void Resize(uint32_t size) {};
		virtual void GetSubData(void** buffer, uint32_t size, uint32_t offset = 0) {};
		virtual void SetLayout(const BufferLayout& layout) {};
		virtual uint32_t GetBinding() const { return 0; }
		virtual const BufferLayout& GetLayout() const { return BufferLayout(); };
		virtual uint32_t GetRendererID() const { return 0; };
		virtual ByteBuffer GetBuffer() { return ByteBuffer(); }

		static Ref<StorageBuffer> Create(uint32_t size, uint32_t binding);

		static Ref<StorageBuffer> Create(const void *data, uint32_t size, uint32_t binding, BufferUsage usage = BufferUsage::Dynamic);
	};


	class AtomicCounter : public RefCount
	{
	public:
		virtual ~AtomicCounter() = default;

		virtual void Reset() = 0;
		virtual void BindBase(uint32_t index) const = 0;
		virtual void Update(uint32_t* data, uint32_t count, uint32_t offset) = 0;
		virtual uint32_t* GetCounters() = 0;
		virtual uint32_t GetNumCounters() = 0;

		static Ref<AtomicCounter> Create(uint32_t size, uint32_t binding);
	};


	struct DrawArraysIndirectCommand
	{
		uint32_t Count;
		uint32_t InstanceCount;
		uint32_t FirstVertex;
		uint32_t BaseInstance;
	};

	struct DrawElementsIndirectCommand
	{
		uint32_t Count;         
		uint32_t InstanceCount; 
		uint32_t FirstIndex;    
		uint32_t BaseVertex;    
		uint32_t BaseInstance;  
	};
	

	class IndirectBuffer : public RefCount
	{
	public:
		virtual ~IndirectBuffer() = default;
		
		virtual void Bind() const = 0;
		virtual void BindBase(uint32_t index) = 0;

		static Ref<IndirectBuffer> Create(void * drawCommand, uint32_t size, uint32_t binding);
	};


	class UniformBuffer : public RefCount
	{
	public:
		virtual ~UniformBuffer() = default;

		virtual void Update(const void* data, uint32_t size, uint32_t offset) = 0;
		virtual void RT_Update(const void* data, uint32_t size, uint32_t offset) {};

		virtual uint32_t GetBinding() const = 0;
		static Ref<UniformBuffer> Create(uint32_t size, uint32_t binding);
	};
}