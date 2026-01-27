#include "engine_core.h"
#include "components.h"

#include "rlText.h"

#include "components/common/transform_component.h"

#include <algorithm>
#include <raylib.h>

using namespace GameObjects;
using namespace Components;

void RenderScene3D(float deltaTime, EngineCore::Scene& scene);

namespace EngineCore
{
    void Init()
    {
       
    }

    float GetDetlaTime()
    {
#if defined(DEBUG)
        return 1.0f / float(GetMonitorRefreshRate(GetCurrentMonitor()));
#endif
        return GetFrameTime();
    }

    void RegisterComponents(Scene& scene)
    {
        scene.RegisterComponentSystem<TransformComponent>();
    }

    void RegisterTasks(Scene& scene)
    {
        scene.AddTask(SceneTaskLevel::Draw3d, RenderScene3D, true, nullptr);
    }

    void Scene::Init()
    {
        RenderTarget = LoadRenderTexture(1920, 1080);
        ViewCamera.fovy = 45;
        ViewCamera.projection = CAMERA_PERSPECTIVE;

        OnInit();

        RegisterComponents(*this);
        OnRegisterComponents();
        RegisterTasks(*this);
        OnRegisterTasks();
    }

    void Scene::Resize(int width, int height)
    {
        UnloadRenderTexture(RenderTarget);
        RenderTarget = LoadRenderTexture(width, height);
    }

    Component* Scene::CreateComponent(size_t id)
    {
        ComponentSystem* sys = GetComponentSystem(id);
        if (!sys)
        {
            return nullptr;
        }
        return sys->Add();
    }

    void Scene::Update()
    {
        ProcessTasks(SceneTaskLevel::PreUpdate);
        ProcessTasks(SceneTaskLevel::Update);
    }

    void Scene::Draw()
    {
        BeginTextureMode(RenderTarget);
        ClearBackground(DARKGRAY);
        ProcessTasks(SceneTaskLevel::RenderTextureCreated);

        DrawDirect();

        ProcessTasks(SceneTaskLevel::RenderTextureFinalized);
        EndTextureMode();
    }

    void Scene::DrawDirect()
    {
        ViewCamera.position = CameraTransform.GetWorldPosition();
        ViewCamera.target = CameraTransform.GetWorldPosition() + CameraTransform.GetWorldForward();
        ViewCamera.up = CameraTransform.GetWorldUp();

        BeginMode3D(ViewCamera);
        ProcessTasks(SceneTaskLevel::PreDraw3d);
        ProcessTasks(SceneTaskLevel::Draw3d);
        ProcessTasks(SceneTaskLevel::PostDraw3d);
        EndMode3D();

        ProcessTasks(SceneTaskLevel::PreDraw2d);
        ProcessTasks(SceneTaskLevel::Draw2d);

        ProcessTasks(SceneTaskLevel::PostDraw2d);
    }

    void Scene::BeginFrame()
    {
        for (auto& taskList : Tasks)
        {
            for (auto& task : taskList.second)
            {
                task->Status = ScenTaskStatus::Waiting;
            }
        }
        ProcessTasks(SceneTaskLevel::First);
    }

    void Scene::EndFrame()
    {
        ProcessTasks(SceneTaskLevel::Final);
    }

    EngineCore::SceneTask* Scene::AddTask(SceneTaskLevel level, std::function<void(float, Scene&)> tick, bool useMainThread /*= true*/, SceneTask* dependsOn /*= nullptr*/)
    {
        std::unique_ptr<SceneTask> task = std::make_unique<SceneTask>();
        task->Tick = tick;

        task->UseMainThread = useMainThread;

        if (dependsOn)
        {
            if (dependsOn->StartOn > level)
            {
                level = dependsOn->StartOn;
            }
        }
        task->StartOn = level;
        task->DependsOn = dependsOn;

        if (dependsOn)
        {
            auto itr = std::find_if(Tasks[level].begin(), Tasks[level].end(), [dependsOn](const auto& arg) {return arg.get() == dependsOn; });
            if (itr != Tasks[level].end())
                itr++;
            Tasks[level].insert(itr, std::move(task));
        }
        else
        {
            Tasks[level].push_back(std::move(task));
        }

        return Tasks[level].back().get();
    }

    void Scene::ProcessTasks(SceneTaskLevel level)
    {
        auto it = Tasks.find(level);
        if (it != Tasks.end())
        {
            for (auto& task : it->second)
            {
                if (!task->Tick)
                {
                    task->Status = ScenTaskStatus::Completed;
                }
                else
                {
                    if (task->UseMainThread)
                    {
                        if (task->DependsOn && task->DependsOn->Status != ScenTaskStatus::Completed)
                        {
                            // wait for the dependency to complete
                            continue;
                        }
                        task->Status = ScenTaskStatus::Started;
                        task->Tick(GetDetlaTime(), *this);
                        task->Status = ScenTaskStatus::Completed;
                    }
                    else
                    {
                        // fire off to a thread pool with the dependency? 
                    }
                }
            }
        }
    }
}
