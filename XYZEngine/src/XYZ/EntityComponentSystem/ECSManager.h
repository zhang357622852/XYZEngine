#pragma once
#include "ComponentManager.h"
#include "EntityManager.h"

namespace XYZ {
	namespace ECS {

		class ECSManager
		{
		public:
			uint32_t CreateEntity() { return m_EntityManager.CreateEntity(); };

			void DestroyEntity(uint32_t entity) 
			{ 
				auto& signature = m_EntityManager.GetSignature(entity);
				signature.set(HAS_GROUP_BIT, false);
				m_ComponentManager.EntityDestroyed(entity, signature, this);
				m_EntityManager.DestroyEntity(entity); 
			}

			template <typename T>
			T& AddComponent(uint32_t entity, const T& component)
			{
				Signature& signature = m_EntityManager.GetSignature(entity);
				XYZ_ASSERT(!signature.test(T::GetComponentID()), "Entity already contains component");
				signature.set(T::GetComponentID(), true);
				auto& result = m_ComponentManager.AddComponent<T>(entity, component);
				if (!signature.test(HAS_GROUP_BIT))
				{
					if (auto group = m_ComponentManager.GetGroup(signature))
						group->AddEntity(entity, signature, this);
				}
				return result;
			}
			
			template <typename T>
			bool RemoveComponent(uint32_t entity)
			{
				Signature& signature = m_EntityManager.GetSignature(entity);
				XYZ_ASSERT(signature.test(T::GetComponentID()), "Entity does not have component");
				signature.set(T::GetComponentID(), false);
				if (signature.test(HAS_GROUP_BIT))
				{
					if (auto group = m_ComponentManager.GetGroup(signature))
						group->RemoveEntity(entity, this);
					signature.set(HAS_GROUP_BIT, false);
				}
				m_ComponentManager.RemoveComponent<T>(entity);
				return true;
			}

			template <typename T>
			T& GetComponent(uint32_t entity)
			{
				Signature& signature = m_EntityManager.GetSignature(entity);
				XYZ_ASSERT(signature.test(T::GetComponentID()), "Entity does not have component");
				if (signature.test(HAS_GROUP_BIT))
				{
					if (auto group = m_ComponentManager.GetGroup(signature))
					{
						uint8_t* component = nullptr;
						group->FindComponent(&component, entity, T::GetComponentID());
						if (component)
							return *(T*)component;
					}
				}
				return m_ComponentManager.GetComponent<T>(entity);
			}

			template <typename T>
			T& GetComponentDirect(uint32_t entity)
			{
				XYZ_ASSERT(m_EntityManager.GetSignature(entity).test(T::GetComponentID()), "Entity does not have component");
				return m_ComponentManager.GetComponent<T>(entity);
			}

			const Signature& GetEntitySignature(uint32_t entity)
			{
				return m_EntityManager.GetSignature(entity);
			}

			bool HasGroup(uint32_t entity) const
			{
				auto& signature = m_EntityManager.GetSignature(entity);
				return signature.test(HAS_GROUP_BIT);
			}

			template <typename T>
			bool Contains(uint32_t entity) const
			{
				auto& signature = m_EntityManager.GetSignature(entity);
				return signature.test(T::GetComponentID());
			}

		
			template <typename T>
			ComponentStorage<T>* GetStorage()
			{
				return (ComponentStorage<T>*)m_ComponentManager.GetStorage(T::GetComponentID());
			}

			template <typename ...Args>
			ComponentGroup<Args...>& GetGroup()
			{
				Signature signature;
				std::initializer_list<uint16_t> componentTypes{ Args::GetComponentID()... };
				for (auto it : componentTypes)
					signature.set(it);

				auto group = m_ComponentManager.GetGroup(signature);
				XYZ_ASSERT(group, "Group does not exist");
				return *(ComponentGroup<Args...>*)group;
			}
			
			template <typename ...Args>
			ComponentGroup<Args...>& CreateGroup()
			{
				return *m_ComponentManager.CreateGroup<Args...>();
			}

			const uint32_t GetNumberOfEntities() const { return m_EntityManager.GetNumEntities(); }

		private:
			ComponentManager m_ComponentManager;
			EntityManager m_EntityManager;
		};
	}
}