#pragma once

#include "object_reference.h"

#include <functional>
#include <vector>
#include <string_view>
#include <memory>
#include <stdexcept>

namespace GameObjects
{
    class GameObject;
}

namespace EngineCore
{
    class Scene;
}

namespace Components
{
#define DEFINE_COMPONENT(T) \
    static size_t ID() {return std::hash<std::string_view>()(#T);} \
    size_t GetID() override { return std::hash<std::string_view>()(#T); } \
    GameObjects::GameObjectReferenceSource<T> ReferenceSource = GameObjects::GameObjectReferenceSource<T>(this); \
    using Reference = std::shared_ptr<GameObjects::GameObjectReference<T>>; \
    Reference GetReference() const { return ReferenceSource.Reference; }

    class Component
    {
    private:
        GameObjects::GameObject * Owner = nullptr;
     
    public:
        virtual size_t GetID() = 0;

        void SetOwner(GameObjects::GameObject* owner) { Owner = owner; }
        GameObjects::GameObject* GetOwner() { return Owner; }
        const GameObjects::GameObject* GetOwner() const{ return Owner; }

        virtual void OnTreeBuildComplete() {}
    };

    class ComponentSystem
    {
    public:
        std::vector<std::unique_ptr<Components::Component>> Components;
        std::function<void(ComponentSystem&, float)> UpdateFunction;
        std::function<std::unique_ptr<Components::Component>()> FactoryFunction;
        size_t ComponentID = 0;

        size_t GetID() const { return ComponentID; }

        template <class T>
        void ForEach(std::function<void(T&, float)> func, float dt = 0)
        {
            for (auto& component : Components)
            {
                if (auto* compType = dynamic_cast<T*>(component.get()))
                    func(*compType, dt);
            }
        }

        Components::Component* Add()
        {
            if (!FactoryFunction)
                throw std::runtime_error("Factory function not set for component system.");

            return Components.emplace_back(std::move(FactoryFunction())).get();
        }

        bool Release(Components::Component* component)
        {
            for (size_t comp = 0; comp < Components.size(); comp++)
            {
                if (Components[comp].get() == component)
                {
                    Components.at(comp) = std::move(Components.back());
                    Components.pop_back();
                    return true; // Successfully released
                }
            }
            return false;
        }
    };
}