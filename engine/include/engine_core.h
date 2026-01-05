
#pragma once

#include "raylib.h"

#include "game_objects.h"
#include "components.h"

#include "components/common/transform_component.h"

#include <memory>
#include <unordered_map>
#include <functional>
#include <stdexcept>

namespace EngineCore
{
    void Init();

    float GetDetlaTime();

    enum class SceneTaskLevel : uint8_t
    {
        First,                  // first thing
        PreUpdate,              // before component updates
        Update,                 // component updates
        RenderTextureCreated,   // render texture has been created and cleared but nothing draw yet
        PreDraw3d,              // 3d mode active, but before scene draw
        Draw3d,                 // 3d scene draw
        PostDraw3d,              // after 3d scene draw (debug scene drawing here)
        PreDraw2d,              // 2d mode active but before GUI and overlays
        Draw2d,                 // 2d GUI and overlays draw
        PostDraw2d,             // after 2d GUi and overlays (debug 2d draw here)

        RenderTextureFinalized,  // scene render texture finalization,
        Final,                  // end of frame                
    };

    enum class ScenTaskStatus
    {
        Waiting = 0,
        Started = 1,
        Completed = 2,
    };

    class SceneTask
    {
    public:
        ScenTaskStatus  Status = ScenTaskStatus::Waiting;

        SceneTaskLevel StartOn = SceneTaskLevel::Update;
        bool UseMainThread = true;

        SceneTask* DependsOn = nullptr;

        std::function<void(float, class Scene&)> Tick;
    };

#define DEFINE_SYSTEM(T) \
    static size_t ID() {return std::hash<std::string_view>()(#T);} \
    size_t GetID() override { return std::hash<std::string_view>()(#T); }


    class SceneSystem
    {
    public:
        class EngineCore::Scene* Scene = nullptr;

        virtual size_t GetID() = 0;
        virtual void OnAttached() {}
        virtual void OnDettached() {}
    };

    class Scene
    {
    private:
        RenderTexture RenderTarget;

        std::unordered_map<size_t, Components::ComponentSystem> ComponentSystems;

        std::unordered_map<size_t, std::unique_ptr<GameObjects::GameObjectTree>> GameObjectRoots;

        std::unordered_map<size_t, std::unique_ptr<SceneSystem>> SceneSystems;

        std::unordered_map<SceneTaskLevel, std::vector<std::unique_ptr<SceneTask>>> Tasks;

    public:
        using Ptr = std::unique_ptr<class Scene>;

        void Init();
        void Resize(int width, int height);

        SceneSystem* RegisterSceneSystem(size_t id, std::unique_ptr<SceneSystem> system)
        {
            if (SceneSystems.find(id) != SceneSystems.end())
                return SceneSystems[id].get();
            system->Scene = this;

            system->OnAttached();
            SceneSystems[id] = std::move(system);

            return SceneSystems[id].get();
        }

        template<class T>
        T* RegisterSceneSystem()
        {
            std::unique_ptr<T> system = std::make_unique<T>();
            return static_cast<T*>(RegisterSceneSystem(T::ID(), std::move(system)));
        }

        SceneSystem* GetSceneSystem(size_t id)
        {
            auto it = SceneSystems.find(id);
            if (it != SceneSystems.end())
                return it->second.get();
            return nullptr;
        }

        template<class T>
        T* GetSceneSystem()
        {
            auto it = SceneSystems.find(T::ID());
            if (it != SceneSystems.end())
                return static_cast<T*>(it->second.get());
            return nullptr;
        }

        void RegisterComponentSystem(size_t id, std::function<void(Components::ComponentSystem&, float)> updateFunc, std::function<std::unique_ptr<Components::Component>()> factoryFunc, SceneTaskLevel startOn = SceneTaskLevel::Update)
        {
            if (ComponentSystems.find(id) != ComponentSystems.end())
                return;

            auto& system = ComponentSystems[id];
            system.UpdateFunction = updateFunc;
            system.FactoryFunction = factoryFunc;

            if (updateFunc)
            {
                // system update functions are stored as tasks
                AddTask(startOn, [id](float dt, Scene& scene)
                    {
                        auto* system = scene.GetComponentSystem(id);
                        if (system)
                            system->UpdateFunction(*system, dt);
                    }, true, nullptr);
            }
        }

        template<class T>
        void RegisterComponentSystem(std::function<void(Components::ComponentSystem&, float)> updateFunc = nullptr, SceneTaskLevel startOn = SceneTaskLevel::Update)
        {
            RegisterComponentSystem(T::ID(), updateFunc, []() { return std::make_unique<T>(); });
        }

        Components::ComponentSystem* GetComponentSystem(size_t id)
        {
            auto it = ComponentSystems.find(id);
            if (it != ComponentSystems.end())
                return &it->second;

            return nullptr;
        }

        template<class T>
        Components::ComponentSystem* GetComponentSystem()
        {
            return GetComponentSystem(T::ID());
        }

        Components::Component* CreateComponent(size_t id);

        template<class T>
        T* CreateComponent()
        {
            auto* system = GetComponentSystem(T::ID());
            if (!system)
            {
                return nullptr;
            }
            return dynamic_cast<T*>(system.Add());
        }

        void ReleaseComponent(Components::Component* component)
        {

        }

        GameObjects::GameObjectTree* CreateGameObjectTree(uint64_t treeId = 0)
        {
            GameObjectRoots[treeId] = std::move(std::make_unique<GameObjects::GameObjectTree>());
            GameObjectRoots[treeId]->Scene = this;
            return GameObjectRoots[treeId].get();
        }

        GameObjects::GameObjectTree* GetGameObjecTree(uint64_t treeId)
        {
            auto itr = GameObjectRoots.find(treeId);
            if (itr == GameObjectRoots.end())
                return nullptr;

            return itr->second.get();
        }

        void Update();

        void Draw();
        void DrawDirect();

        void BeginFrame();
        void EndFrame();

        SceneTask* AddTask(SceneTaskLevel level, std::function<void(float, Scene&)> tick, bool useMainThread = true, SceneTask* dependsOn = nullptr);

        void ProcessTasks(SceneTaskLevel level);

        RenderTexture& GetRenderTarget() { return RenderTarget; }

        GameObjects::TransformComponent CameraTransform;
        Camera3D ViewCamera = { 0 };
    };

    std::unique_ptr<Scene> CreateScene();
}
