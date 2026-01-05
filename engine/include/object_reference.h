#pragma once

#include <memory>

namespace GameObjects
{

    /// <summary>
    /// A reference to a game object with a lifetime token
    /// </summary>
    /// <typeparam name="T"></typeparam>
    template<class T>
    class GameObjectReference
    {
    private:
        T* Object = nullptr;

    public:
        GameObjectReference(T* object)
            : Object(object)
        {
        }

        T* GetObject() const { return Object; }

        template <class U>
        U* GetObjectAs() const { return reinterpret_cast<U*>(Object); }

        bool IsValid() const { return Object != nullptr; }

        template <class U>
        bool IsValid() const { return reinterpret_cast<U*>(Object) != nullptr; }

        void Invalidate() { Object = nullptr; }
    };

    template<class T>
    class GameObjectReferenceSource
    {
    public:
        std::shared_ptr<GameObjectReference<T>> Reference;

        GameObjectReferenceSource(T* object)
        {
            Reference = std::make_shared<GameObjectReference<T>>(object);
        }

        ~GameObjectReferenceSource()
        {
            Reference->Invalidate();
        }

    };
}