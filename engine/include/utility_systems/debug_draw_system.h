#pragma once

#include "components.h"
#include "components/common/transform_component.h"
#include "engine_core.h"

#include <vector>

namespace DebugDraw
{
    enum class DebugDrawType
    {
        None,
        AxisLine,
        AxisMarker,
    };

    class DebugDrawSystem : public EngineCore::SceneSystem
    {
    private:
        struct ObjectInfo
        {
            GameObjects::TransformComponent::Reference transform;
            float size;
            bool useOrientation;
        };
        std::vector<ObjectInfo> AxisLines;
        std::vector<ObjectInfo> AxisMarkers;

    private:
        void Draw3D(float deltaTime, const EngineCore::Scene& scene);
        void Draw2D(float deltaTime, const EngineCore::Scene& scene);

        void Reset();

        void DrawAxisLines(const ObjectInfo& scene);
        void DrawAxisMarker(const ObjectInfo& scene);

    public:
        DEFINE_SYSTEM(DebugDrawSystem);

        bool Enabled = true;

        void OnAttached() override;

        void AddAxisLines(GameObjects::TransformComponent::Reference transform, float size = 1, bool useOrientation = true);
        void AddAxisMarker(GameObjects::TransformComponent::Reference transform, float size = 1, bool useOrientation = true);
    };

    class DebugDrawShapeComponent : public Components::Component
    {
    public:
        DEFINE_COMPONENT(DebugDrawShapeComponent);
        DebugDrawType ShapeType = DebugDrawType::None;
        Vector3 Size = { 1.0f, 1.0f, 1.0f };
    };
}