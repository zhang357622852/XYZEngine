#pragma once
#include "XYZ/Core/Core.h"

#include <mutex>
#include <deque>

namespace XYZ {
	template <typename T>
	class XYZ_API ThreadQueue
	{
	public:
		ThreadQueue() = default;
		ThreadQueue(const ThreadQueue<T>&) = delete;

		const T& Front()
		{
			std::scoped_lock lock(m_Mutex);
			return m_Queue.front();
		}

		const T& Back()
		{
			std::scoped_lock lock(m_Mutex);
			return m_Queue.back();
		}

		void PushBack(const T& elem)
		{
			std::scoped_lock lock(m_Mutex);
			m_Queue.push_back(elem);
		}

		void PushFront(const T& elem)
		{
			std::scoped_lock lock(m_Mutex);
			m_Queue.push_front(elem);
		}

		void EmplaceBack(T&& elem)
		{
			std::scoped_lock lock(m_Mutex);
			m_Queue.emplace_back(std::move(elem));
		}

		void EmplaceFront(T&& elem)
		{
			std::scoped_lock lock(m_Mutex);
			m_Queue.emplace_front(std::move(elem));
		}

		bool Empty()
		{
			std::scoped_lock lock(m_Mutex);
			return m_Queue.empty();
		}

		size_t Size()
		{
			std::scoped_lock lock(m_Mutex);
			return m_Queue.size();
		}

		void Clear()
		{
			std::scoped_lock lock(m_Mutex);
			m_Queue.clear();
		}

		T PopFront()
		{
			std::scoped_lock lock(m_Mutex);
			T temp = std::move(m_Queue.front());
			m_Queue.pop_front();
			return temp;
		}

		T PopBack()
		{
			std::scoped_lock lock(m_Mutex);
			T temp = std::move(m_Queue.back());
			m_Queue.pop_back();
			return temp;
		}


		std::mutex& GetMutex() { return m_Mutex; }

	private:
		std::deque<T> m_Queue;
		std::mutex m_Mutex;
	};
}