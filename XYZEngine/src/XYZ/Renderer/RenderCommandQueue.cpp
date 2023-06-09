#include "stdafx.h"
#include "RenderCommandQueue.h"

#include <assert.h>

namespace XYZ {
	RenderCommandQueue::RenderCommandQueue(uint32_t size)
		:
		m_Size(size)
	{
		m_CommandBuffer = new uint8_t[size];
		m_CommandBufferPtr = m_CommandBuffer;
		memset(m_CommandBuffer, 0, size);
	}

	RenderCommandQueue::~RenderCommandQueue()
	{
		delete[] m_CommandBuffer;
	}

	void* RenderCommandQueue::Allocate(RenderCommandFn fn, uint32_t size)
	{
		XYZ_ASSERT(m_CommandBufferPtr - m_CommandBuffer < m_Size - size, "Command queue out of range");
		*(RenderCommandFn*)m_CommandBufferPtr = fn;
		m_CommandBufferPtr += sizeof(RenderCommandFn);

		*(uint32_t*)m_CommandBufferPtr = size;
		m_CommandBufferPtr += sizeof(uint32_t);

		void* memory = m_CommandBufferPtr;
		m_CommandBufferPtr += size;

		m_CommandCount++;
		return memory;
	}


	void RenderCommandQueue::Execute()
	{
		//std::scoped_lock<std::mutex> lock(m_Mutex);
		uint8_t* buffer = m_CommandBuffer;
		for (uint32_t i = 0; i < m_CommandCount; i++)
		{
			const RenderCommandFn function = *(RenderCommandFn*)buffer;
			buffer += sizeof(RenderCommandFn);

			const uint32_t size = *(uint32_t*)buffer;
			buffer += sizeof(uint32_t);
			function(buffer);
			buffer += size;
		}

		m_CommandBufferPtr = m_CommandBuffer;
		m_CommandCount = 0;
	}
}