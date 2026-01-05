#include "game_objects.h"

#include "components.h"
#include "engine_core.h"

namespace GameObjects
{
    GameObject::~GameObject()
    {
        Reference->Invalidate();
        for (auto& [id, component] : Components)
        {
            Scene->ReleaseComponent(component);
        }
    }

    Components::Component* GameObject::CreateComponent(size_t id)
    {
        if (!Scene)
            return nullptr;

        auto* component = Scene->CreateComponent(id);
        if (component)
        {
            Components[id] = component;
            component->SetOwner(this); // Set the owner of the component
        }
        return component;
    }

}