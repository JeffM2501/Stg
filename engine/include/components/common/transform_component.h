#pragma once

#include "components.h"
#include "raylib.h"
#include "raymath.h"

#include <vector>

namespace GameObjects
{
    class TransformComponent : public Components::Component
    {
    public:
        DEFINE_COMPONENT(TransformComponent);

    private:
        Vector3 Position = { 0 };
        Quaternion Orientation = QuaternionIdentity();

        bool Dirty = true;

        Matrix LocalMatrix = MatrixIdentity();
        Matrix WorldMatrix = MatrixIdentity();
        Quaternion WorldOrientation = QuaternionIdentity();

        Matrix GlWorldMatrix = MatrixIdentity();

        TransformComponent::Reference Parent;

        std::vector<TransformComponent::Reference> Children;

        void SetDirty();
        void ValidateMatrix();

    public:
        void OnTreeBuildComplete() override;

        void SetPosition(const Vector3& position);
        void SetPosition(float x, float y, float z);
        void SetOrientation(const Vector3& orientation);
        void SetOrientation(const Quaternion& orientation);

        Vector3 GetPosition() const;
        Vector3 GetEulerAngles() const;

        Vector3 GetWorldPosition();
        Vector3 GetWorldEulerAngles();
        Vector3 GetWorldOrientation();

        Vector3 GetForward();
        Vector3 GetUp();
        Vector3 GetRight();

        Vector3 GetWorldForward();
        Vector3 GetWorldUp();
        Vector3 GetWorldRight();

        void MoveForward(float delta);
        void MoveUp(float delta);
        void MoveRight(float delta);

        void AttachChild(TransformComponent* newChild);
        void DetatchChild(TransformComponent* newChild);
        void RemoveParent();

        enum class Space
        {
            Local,
            World
        };

        void RotateX(float angle, Space space = Space::Local);
        void RotateY(float angle, Space space = Space::Local);
        void RotateZ(float angle, Space space = Space::Local);

        void LookAt(const Vector3& target, const Vector3& up);

        Matrix GetWorldMatrix();

        Transform GetWorldTransform()
        {
            ValidateMatrix();
            return Transform{ Vector3{ WorldMatrix.m12, WorldMatrix.m13, WorldMatrix.m14 }, QuaternionFromMatrix(WorldMatrix), Vector3Ones };
        }

        void PushMatrix();

        void PopMatrix();
    };
}