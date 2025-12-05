//
// Created by Sanjay on 7/19/2025.
//

#pragma once
#include "Math.h"
#include <vector>

class Actor;

class Transform
{
public:
	// Getters
	Vector3 GetPosition();
	float GetRotation() const;
	const Vector3& GetScale() const;
	const Quaternion& GetQuat() const;

	Actor* GetParent() const { return mParent; }
	const std::vector<class Actor*>& GetChildren() const { return mChildren; }

	// Setters
	void SetPosition(const Vector3& position);
	void SetRotation(float rotation);
	void SetScale(float scale);
	void SetScale(const Vector3& scale);
	void SetQuat(const Quaternion& quat);

	// Movement
	Vector3 GetForward();
	Vector3 GetRight();

	// World Transform
	const Matrix4& GetWorldTransform();

	// Parenting
	void DirtyTransform();
	void SetupParent(class Actor* self, class Actor* parent);

private:
	Vector3 mPosition;				  // Center point of actor
	float mRotation = 0.0f;			  // Rotation of the actor
	Vector3 mScale{1.0f, 1.0f, 1.0f}; // Scale of the actor

	// World Transform Variables
	Matrix4 mWorldTransform;		// World transform matrix
	bool mIsTransformDirty = false; // Tracks if transform needs recalculating

	Quaternion mQuat;

	class Actor* mParent = nullptr;
	std::vector<class Actor*> mChildren;
};
