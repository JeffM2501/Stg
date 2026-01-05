#pragma once

#include "object_reference.h"

#include <memory>
#include <vector>
#include <functional>
#include <unordered_map>

namespace EngineCore
{
    class Scene;
}

namespace Components
{
    class Component;
}

namespace GameObjects
{
    // an entity in the game, with children and a list of components
    class GameObject
    {
    private:
        friend class GameObjectTree;
        EngineCore::Scene* Scene = nullptr;

        std::shared_ptr<GameObjectReference<GameObject>> Reference = nullptr;

        GameObject* Parent = nullptr;
        std::vector<GameObject*> Children;

        size_t EntityID = 0;

        std::unordered_map<size_t, Components::Component*> Components;

    public:
        GameObject(EngineCore::Scene* scene)
            : Scene(scene)
            ,Reference(std::make_shared<GameObjectReference<GameObject>>(this))
        {
        }

        virtual ~GameObject();

        GameObject* GetParent() { return Parent; }
        const std::vector<GameObject*> GetChildren() { return Children; }

        std::shared_ptr<GameObjectReference<GameObject>> GetReference() const { return Reference; }

        void AddChild(GameObject* child)
        {
            if (child)
            {
                child->Parent = this;
                Children.push_back(child);
            }
        }

        Components::Component* CreateComponent(size_t id);

        template <class T>
        T* CreateComponent()
        {
            if (!Scene)
                return nullptr;

            return dynamic_cast<T*>(CreateComponent(T::ID()));
        }

        template <class T>
        T* GetComponent(bool craateIfNotExists = false)
        {
            auto it = Components.find(T::ID());
            if (it != Components.end())
            {
                return static_cast<T*>(it->second);
            }
            if (craateIfNotExists)
                return CreateComponent<T>();

            return nullptr;
        }

        template <class T>
        bool HasComponent() const
        {
            return Components.find(T::ID()) != Components.end();
        }

        template <class T>
        void ForEachChildComponent(std::function<void(T*)> callback, bool recursive = false)
        {
            if (!callback)
                return;

            for (GameObject* child : Children)
            {
                T* component = child->GetComponent<T>();
                if (component)
                    callback(component);
        
                if (recursive)
                    child->ForEachChildComponent<T>(callback, recursive);
            }
        }
    };

    // a list of game objects linked up in a hierarchical tree, but stored flat
    class GameObjectTree
    {
    public:
        std::vector<std::unique_ptr<GameObject>> Objects;
        size_t ID = 0;

        EngineCore::Scene* Scene = nullptr;

        GameObject* CreateGameObject(size_t entityID = size_t(-1), GameObject* parent = nullptr)
        {
            auto object = std::make_unique<GameObject>(Scene);
            if (entityID != size_t(-1))
                object->EntityID = entityID;

            GameObject* rawPtr = object.get();
            if (parent)
                parent->AddChild(rawPtr);
            Objects.push_back(std::move(object));

            return rawPtr;
        }
    };

}