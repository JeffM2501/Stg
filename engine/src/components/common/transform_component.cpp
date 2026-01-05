
#include "components/common/transform_component.h"
#include "game_objects.h"
#include "raymath.h"
#include "rlgl.h"

namespace GameObjects
{
    void TransformComponent::OnTreeBuildComplete()
    {
        Parent = nullptr;
        Children.clear();

        GameObject* owner = GetOwner();
        if (!owner)
            return;

        // get references to the parent and children
        if (owner->GetParent())
        {
            Parent = owner->GetComponent<TransformComponent>()->GetReference();
        }

        owner->ForEachChildComponent<TransformComponent>([this](TransformComponent* child)
            {
                Children.push_back(child->GetReference());
            }, false);
    }

    void TransformComponent::SetPosition(const Vector3& position)
    {
        Position = position;
        SetDirty();
    }
    
    void TransformComponent::SetPosition(float x, float y, float z)
    {
        Position = Vector3{ x,y,z };
        SetDirty();
    }

    void TransformComponent::SetOrientation(const Vector3& orientation)
    {
        Orientation = QuaternionFromEuler(orientation.x, orientation.y, orientation.z);
        SetDirty();
    }

    void TransformComponent::SetOrientation(const Quaternion& orientation)
    {
        Orientation = orientation;
        SetDirty();
    }

    Vector3 TransformComponent::GetPosition() const
    {
        return Position;
    }

    Vector3 TransformComponent::GetEulerAngles() const
    {
        return QuaternionToEuler(Orientation);
    }

    Vector3 TransformComponent::GetWorldPosition()
    {
        ValidateMatrix();
        return Vector3{ WorldMatrix.m12, WorldMatrix.m13, WorldMatrix.m14 };
    }

    Vector3 TransformComponent::GetWorldEulerAngles()
    {
        ValidateMatrix();
        return QuaternionToEuler(QuaternionFromMatrix(WorldMatrix));
    }

    Vector3 TransformComponent::GetWorldOrientation()
    {
        ValidateMatrix();
        return QuaternionToEuler(QuaternionFromMatrix(WorldMatrix));
    }

    Vector3 TransformComponent::GetForward()
    {
        ValidateMatrix();
        return Vector3RotateByQuaternion(Vector3UnitX, QuaternionInvert(Orientation));
    }

    Vector3 TransformComponent::GetUp()
    {
        ValidateMatrix();
        return Vector3RotateByQuaternion(Vector3UnitZ, QuaternionInvert(Orientation));
    }

    Vector3 TransformComponent::GetRight()
    {
        ValidateMatrix();
        return Vector3RotateByQuaternion(Vector3UnitY * -1.0f, QuaternionInvert(Orientation));
    }

    Vector3 TransformComponent::GetWorldForward()
    {
        ValidateMatrix(); 
        return Vector3RotateByQuaternion(Vector3UnitX, WorldOrientation);
    }

    Vector3 TransformComponent::GetWorldUp()
    {
        ValidateMatrix();
        return Vector3RotateByQuaternion(Vector3UnitZ, WorldOrientation);
    }

    Vector3 TransformComponent::GetWorldRight()
    {
        ValidateMatrix();
        return Vector3RotateByQuaternion(Vector3UnitY, WorldOrientation);
    }

    void TransformComponent::AttachChild(TransformComponent* newChild)
    {
        if (!newChild)
            return;

        if (newChild->Parent && newChild->Parent->IsValid() && newChild->Parent->GetObject() == this)
            return;

        Children.push_back(newChild->GetReference());
        newChild->Parent = GetReference();
        newChild->SetDirty();

        // TODO, keep worldspace transform?
    }

    void TransformComponent::RemoveParent()
    {
        if (Parent && Parent->IsValid() && Parent->GetObject())
        {
            Parent->GetObject()->DetatchChild(this);
        }
        // todo, bake in the world transform?
        Parent = nullptr;
    }

    void TransformComponent::DetatchChild(TransformComponent* newChild)
    {
        for (auto childItr = Children.begin(); childItr != Children.end();)
        {
            if (!(*childItr)->IsValid<TransformComponent>())
            {
                childItr = Children.erase(childItr);
                continue;
            }

            if ((*childItr)->GetObject() == newChild)
            {
                childItr = Children.erase(childItr);
                newChild->Parent.reset();
                newChild->SetDirty();
                // todo, bake in the world transform?
                return;
            }

            childItr++;
        }
    }

    void TransformComponent::RotateX(float angle, Space space)
    {
        auto rot = QuaternionFromEuler(angle * DEG2RAD, 0, 0);
        if (space == Space::Local)
            Orientation = QuaternionMultiply(rot, Orientation);
        else
            Orientation = QuaternionMultiply(Orientation, rot);
        SetDirty();
    }

    void TransformComponent::RotateY(float angle, Space space)
    {
        auto rot = QuaternionFromEuler(0, angle * DEG2RAD, 0);
        if (space == Space::Local)
            Orientation = QuaternionMultiply(rot, Orientation);
        else
            Orientation = QuaternionMultiply(Orientation, rot);
        SetDirty();
    }

    void TransformComponent::RotateZ(float angle, Space space)
    {
        auto rot = QuaternionFromEuler(0, 0, angle * DEG2RAD);
        if (space == Space::Local)
            Orientation = QuaternionMultiply(rot, Orientation);
        else
            Orientation = QuaternionMultiply(Orientation, rot);
        SetDirty();
    }

    void TransformComponent::MoveForward(float delta)
    {
        Position += GetForward() * delta;
        SetDirty();
    }

    void TransformComponent::MoveUp(float delta)
    {
        Position += GetUp() * delta;
        SetDirty();
    }

    void TransformComponent::MoveRight(float delta)
    {
        Position += GetRight() * delta;
        SetDirty();
    }

    void TransformComponent::LookAt(const Vector3& target, const Vector3& up)
    {
        auto pos = GetWorldPosition();

        // look at the target in world space
        Matrix mat = MatrixLookAt(pos, target, up);

      
        // get the parent matrix, so we can convert to local space
        Matrix parentMatrix = MatrixIdentity();
        if(Parent && Parent->IsValid())
            parentMatrix = Parent->GetObject()->GetWorldMatrix();

        parentMatrix = MatrixInvert(parentMatrix);
        mat = MatrixMultiply(mat, parentMatrix);

        // get the local space orientation to look at that worldspace target
        Orientation = QuaternionFromMatrix(mat);

        auto foward = Vector3RotateByQuaternion(Vector3UnitX, Orientation);

        SetDirty();
    }

    void TransformComponent::SetDirty()
    {
        Dirty = true;

        for (auto childItr = Children.begin(); childItr != Children.end();)
        {
            if (!(*childItr)->IsValid<TransformComponent>())
            {
                childItr = Children.erase(childItr);
                continue;
            }

            (*childItr)->GetObjectAs<TransformComponent>()->SetDirty();
            childItr++;
        }  
    }

    void TransformComponent::ValidateMatrix()
    {
        if (Dirty)
        {
            Matrix orient = QuaternionToMatrix(Orientation);
            Matrix translation = MatrixTranslate(Position.x, Position.y, Position.z);

            LocalMatrix = MatrixMultiply(MatrixInvert(orient), translation);

            Matrix parentMatrix = MatrixIdentity();
            if (Parent)
            {
                if (!Parent->IsValid())
                    Parent = nullptr;
                else
                    parentMatrix = Parent->GetObject()->GetWorldMatrix();
            }

            WorldMatrix = MatrixMultiply(LocalMatrix, parentMatrix);
            WorldOrientation = QuaternionFromMatrix(WorldMatrix);
            GlWorldMatrix = MatrixTranspose(WorldMatrix);

            Dirty = false;
        }
    }

    Matrix TransformComponent::GetWorldMatrix()
    {
        ValidateMatrix();
        return WorldMatrix;
    }

    void TransformComponent::PushMatrix()
    {
        ValidateMatrix();
        rlPushMatrix();
        rlMultMatrixf((float*)(&GlWorldMatrix.m0));
    }

    void TransformComponent::PopMatrix()
    {
        rlPopMatrix();
    }
}