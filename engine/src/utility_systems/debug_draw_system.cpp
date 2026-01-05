#include "utility_systems/debug_draw_system.h"

#include "rlgl.h"

using namespace EngineCore;
using namespace GameObjects;

namespace DebugDraw
{
    void DebugDrawAxisLines(const Vector3& position, float size = 1)
    {
        DrawLine3D(position, Vector3Add(position, Vector3Scale(Vector3UnitX, size)), RED);   // X axis
        DrawLine3D(position, Vector3Add(position, Vector3Scale(Vector3UnitY, size)), GREEN);     // Y axis
        DrawLine3D(position, Vector3Add(position, Vector3Scale(Vector3UnitZ, size)), BLUE);  // Z axis
    }

    void DebugDrawAxisMarker(const Vector3& position, float size = 1)
    {
        rlPushMatrix();
        rlTranslatef(position.x, position.y, position.z);

        rlBegin(RL_QUADS);
        rlSetTexture(rlGetTextureIdDefault());

        auto defaultTeture = GetShapesTexture();
        auto rect = GetShapesTextureRectangle();
        rlTexCoord2f(float(rect.x) / defaultTeture.width, float(rect.y) / defaultTeture.height);

        float axisWidth = size * 0.075f;
        float arowThick = size * 0.25f;

        rlNormal3f(0, 0, 1);
        rlColor4f(1, 0, 0, 1);

        auto drawArrow = [&]()
            {
                rlVertex3f(axisWidth, axisWidth, 0);
                rlVertex3f(axisWidth, -axisWidth, 0);
                rlVertex3f(size - arowThick, -axisWidth, 0);
                rlVertex3f(size - arowThick, axisWidth, 0);

                rlVertex3f(size - arowThick, 0, 0);
                rlVertex3f(size - arowThick, -axisWidth * 3, 0);
                rlVertex3f(size, 0, 0);
                rlVertex3f(size - arowThick, axisWidth * 3, 0);

                rlVertex3f(axisWidth, -axisWidth, 0);
                rlVertex3f(axisWidth, axisWidth, 0);
                rlVertex3f(size - arowThick, axisWidth, 0);
                rlVertex3f(size - arowThick, -axisWidth, 0);

                rlVertex3f(size - arowThick, 0, 0);
                rlVertex3f(size - arowThick, axisWidth * 3, 0);
                rlVertex3f(size, 0, 0);
                rlVertex3f(size - arowThick, -axisWidth * 3, 0);
            };

        drawArrow();
        rlPushMatrix();
        rlRotatef(90, 0, 0, 1);
        rlColor4f(0, 1, 0, 1);
        drawArrow();
        rlPopMatrix();

        rlPushMatrix();

        rlRotatef(-90, 0, 1, 0);
        rlRotatef(45, 1, 0, 0);
        rlColor4f(0, 0, 1, 1);
        drawArrow();
        rlPopMatrix();

        rlPopMatrix();
    }

    void DebugDrawSystem::Draw3D(float deltaTime, const EngineCore::Scene& scene)
    {
        if (!Enabled)
            return;

        auto addAxisLines = [this](DebugDrawShapeComponent& shape, float size)
            {
                auto* transform = shape.GetOwner()->GetComponent<TransformComponent>();
                if (!transform)
                    return;

                switch (shape.ShapeType)
                {
                case DebugDrawType::AxisLine:
                    AddAxisLines(transform->GetReference(), shape.Size.x, true);
                    break;

                case DebugDrawType::AxisMarker:
                    AddAxisMarker(transform->GetReference(), shape.Size.x, true);
                    break;
                default:
                    break;
                }

            };
        Scene->GetComponentSystem<DebugDrawShapeComponent>()->ForEach<DebugDrawShapeComponent>(addAxisLines);

        for (const auto& line : AxisLines)
            DrawAxisLines(line);

        for (const auto& marker : AxisMarkers)
            DrawAxisMarker(marker);
    }

    void DebugDrawSystem::Draw2D(float deltaTime, const EngineCore::Scene& scene)
    {
        if (!Enabled)
            return;
    }

    void DebugDrawSystem::Reset()
    {
        AxisLines.clear();
        AxisMarkers.clear();
    }

    void DebugDrawSystem::DrawAxisLines(const ObjectInfo& line)
    {
        if (!line.transform->IsValid())
            return;

        if (line.useOrientation)
        {
            line.transform->GetObjectAs<GameObjects::TransformComponent>()->PushMatrix();
            DebugDrawAxisLines(Vector3Zeros, line.size);
            line.transform->GetObjectAs<GameObjects::TransformComponent>()->PopMatrix();
        }
        else
        {
            DebugDrawAxisLines(line.transform->GetObjectAs<GameObjects::TransformComponent>()->GetWorldPosition(), line.size);
        }
    }

    void DebugDrawSystem::DrawAxisMarker(const ObjectInfo& marker)
    {
        if (!marker.transform->IsValid())
            return;

        if (marker.useOrientation)
        {
            marker.transform->GetObjectAs<GameObjects::TransformComponent>()->PushMatrix();
            DebugDrawAxisMarker(Vector3Zeros, marker.size);
            marker.transform->GetObjectAs<GameObjects::TransformComponent>()->PopMatrix();
        }
        else
        {
            DebugDrawAxisMarker(marker.transform->GetObjectAs<GameObjects::TransformComponent>()->GetWorldPosition(), marker.size);
        }
    }

    void DebugDrawSystem::OnAttached()
    {
        Scene->AddTask(SceneTaskLevel::First, [this](float deltaTime, EngineCore::Scene& scene) { Reset(); });

        Scene->AddTask(SceneTaskLevel::PostDraw3d, [this](float deltaTime, EngineCore::Scene& scene) { Draw3D(deltaTime, scene); });
        Scene->AddTask(SceneTaskLevel::PostDraw2d, [this](float deltaTime, EngineCore::Scene& scene) { Draw2D(deltaTime, scene); });

        Scene->RegisterComponentSystem<DebugDrawShapeComponent>();
    }

    void DebugDrawSystem::AddAxisLines(GameObjects::TransformComponent::Reference transform, float size /*= 1*/, bool useOrientation /*= true*/)
    {
        AxisLines.push_back(ObjectInfo{ transform, size, useOrientation });
    }

    void DebugDrawSystem::AddAxisMarker(GameObjects::TransformComponent::Reference transform, float size /*= 1*/, bool useOrientation /*= true*/)
    {
        AxisMarkers.push_back(ObjectInfo{ transform, size, useOrientation });
    }

}