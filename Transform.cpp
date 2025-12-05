//
// Created by Sanjay on 7/19/2025.
//
#include "Transform.h"
#include "Actor.h"

// Getters
Vector3 Transform::GetPosition()
{
	return GetWorldTransform().GetTranslation();
}

float Transform::GetRotation() const
{
	return mRotation;
}

const Vector3& Transform::GetScale() const
{
	return mScale;
}

const Quaternion& Transform::GetQuat() const
{
	return mQuat;
}

// Setters
void Transform::SetPosition(const Vector3& position)
{
	mPosition = position;
	DirtyTransform();
}

void Transform::SetRotation(float rotation)
{
	mRotation = rotation;
	DirtyTransform();
}

void Transform::SetScale(float scale)
{
	mScale = {scale, scale, scale};
	DirtyTransform();
}

void Transform::SetScale(const Vector3& scale)
{
	mScale = scale;
	DirtyTransform();
}

void Transform::SetQuat(const Quaternion& quat)
{
	mQuat = quat;
	DirtyTransform();
}

Vector3 Transform::GetForward()
{
	// +x is our forward, so we want our x-axis
	return GetWorldTransform().GetXAxis();
}

Vector3 Transform::GetRight()
{
	// y is the actor's right vector
	return GetWorldTransform().GetYAxis();
}

const Matrix4& Transform::GetWorldTransform()
{
	if (mIsTransformDirty)
	{
		mIsTransformDirty = false;

		Matrix4 matScale = Matrix4::CreateScale(mScale);
		Matrix4 matRotZ = Matrix4::CreateRotationZ(mRotation);
		Matrix4 matQuat = Matrix4::CreateFromQuaternion(mQuat);
		Matrix4 matTrans = Matrix4::CreateTranslation(mPosition);

		// World = Scale * RotationZ * Quaternion * Translation
		mWorldTransform = matScale * matRotZ * matQuat * matTrans;

		// Apply parent world transform if we have a parent
		if (mParent)
		{
			mWorldTransform *= mParent->GetTransform().GetWorldTransform();
		}
	}
	return mWorldTransform;
}

void Transform::DirtyTransform()
{
	mIsTransformDirty = true;

	for (class Actor* child : mChildren)
	{
		if (child)
		{
			child->GetTransform().DirtyTransform();
		}
	}
}

void Transform::SetupParent(class Actor* self, class Actor* parent)
{
	mParent = parent;

	if (parent)
	{
		parent->GetTransform().mChildren.emplace_back(self);
	}

	DirtyTransform();
}
