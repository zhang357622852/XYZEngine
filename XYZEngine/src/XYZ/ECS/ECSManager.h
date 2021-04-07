#pragma once
#include "ComponentManager.h"
#include "EntityManager.h"
#include "CallbackManager.h"
#include "ComponentView.h"

namespace XYZ {
	 
	class ECSManager
	{
	public:
		ECSManager() = default;
		ECSManager(const ECSManager& other);
		ECSManager(ECSManager&& other) noexcept;

		ECSManager& operator=(ECSManager&& other) noexcept;

		uint32_t CreateEntity() { return m_EntityManager.CreateEntity(); };

		void DestroyEntity(Entity entity) 
		{ 
			auto& signature = m_EntityManager.GetSignature(entity);
			m_CallbackManager.OnEntityDestroyed(entity, signature);
			m_ComponentManager.EntityDestroyed(entity, signature);
			m_EntityManager.DestroyEntity(entity); 
		}

		template <typename T>
		void AddListener(const std::function<void(uint32_t, CallbackType)>& callback, void* instance)
		{
			m_CallbackManager.AddListener<T>(callback, instance);
		}

		template <typename T>
		void RemoveListener(void* instance)
		{
			m_CallbackManager.RemoveListener<T>(instance);
		}

		template <typename T, typename ...Args>
		T& EmplaceComponent(Entity entity, Args&&... args)
		{
			Signature& signature = m_EntityManager.GetSignature(entity);
			XYZ_ASSERT(!signature.test(IComponent::GetComponentID<T>()), "Entity already contains component");
			signature.set(IComponent::GetComponentID<T>(), true);
			auto& result = m_ComponentManager.EmplaceComponent<T>(entity, std::forward<Args>(args)...);
			m_CallbackManager.OnComponentCreate<T>(entity);

			return result;
		}
		template <typename T>
		T& AddComponent(Entity entity, const T& component)
		{
			Signature& signature = m_EntityManager.GetSignature(entity);
			XYZ_ASSERT(!signature.test(IComponent::GetComponentID<T>()), "Entity already contains component");
			signature.set(IComponent::GetComponentID<T>(), true);
			auto& result = m_ComponentManager.AddComponent<T>(entity, component);
			m_CallbackManager.OnComponentCreate<T>(entity);

			return result;
		}
		
		template <typename T>
		bool RemoveComponent(Entity entity)
		{
			Signature& signature = m_EntityManager.GetSignature(entity);
			XYZ_ASSERT(signature.test(IComponent::GetComponentID<T>()), "Entity does not have component");
			
			signature.set(IComponent::GetComponentID<T>(), false);
			m_CallbackManager.OnComponentRemove<T>(entity);
			m_ComponentManager.RemoveComponent<T>(entity, signature);

			return true;
		}

		template <typename T>
		T& GetComponent(Entity entity)
		{
			Signature& signature = m_EntityManager.GetSignature(entity);
			XYZ_ASSERT(signature.test(IComponent::GetComponentID<T>()), "Entity does not have component");
			return m_ComponentManager.GetComponent<T>(entity);
		}

		template <typename T>
		const T& GetComponent(Entity entity) const
		{
			const Signature& signature = m_EntityManager.GetSignature(entity);
			XYZ_ASSERT(signature.test(IComponent::GetComponentID<T>()), "Entity does not have component");
			return m_ComponentManager.GetComponent<T>(entity);
		}


		const Signature& GetEntitySignature(Entity entity) const
		{
			return m_EntityManager.GetSignature(entity);
		}

		template <typename T>
		bool Contains(Entity entity) const
		{
			auto& signature = m_EntityManager.GetSignature(entity);
			return signature.test(IComponent::GetComponentID<T>());
		}

		bool IsValid(Entity entity) const
		{
			return m_EntityManager.m_Valid.size() > (uint32_t)entity && m_EntityManager.m_Valid[(uint32_t)entity];
		}

		template <typename T>
		void ForceStorage()
		{
			m_ComponentManager.ForceStorage<T>();
		}

		template <typename T>
		ComponentStorage<T>& GetStorage()
		{
			return m_ComponentManager.GetStorage<T>();
		}

		template <typename T>
		const ComponentStorage<T>& GetStorage() const
		{
			return m_ComponentManager.GetStorage<T>();
		}

		template <typename ...Args>
		ComponentView<Args...> CreateView()
		{
			return ComponentView<Args...>(m_ComponentManager);
		}

		template <typename T>
		uint32_t GetComponentIndex(Entity entity) const
		{
			return m_ComponentManager.GetComponentIndex<T>(entity);
		}

		template <typename T>
		Entity FindEntity(const T& component) const
		{
			for (int32_t i = 0; i < m_EntityManager.m_Signatures.Range();++i)
			{
				if (m_EntityManager.m_Signatures[i].test(IComponent::GetComponentID<T>()))
				{
					if (component == m_ComponentManager.GetComponent<T>(i))
						return i;
				}
			}
			return Entity();
		}

		const uint32_t GetNumberOfEntities() const { return m_EntityManager.GetNumEntities(); }

		size_t GetNumberOfRegisteredComponentTypes() const { return m_ComponentManager.GetNumberOfRegisteredStorages(); }
	private:
		ComponentManager m_ComponentManager;
		CallbackManager m_CallbackManager;
		EntityManager m_EntityManager;
	};
	
}