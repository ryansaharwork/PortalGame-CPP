//
// Created by Ryan Sahar on 10/29/25.
//
#include "CameraComponent.h"
#include "Actor.h"
#include "Game.h"
#include "Renderer.h"

CameraComponent::CameraComponent(class Actor* owner)
: Component(owner)
{
}

void CameraComponent::HandleUpdate(float deltaTime)
{
	// Update pitch
	mPitchAngle += mPitchSpeed * deltaTime;

	// Clamp pitch angle
	if (mPitchAngle < -MAX_PITCH_ANGLE)
	{
		mPitchAngle = -MAX_PITCH_ANGLE;
	}
	else if (mPitchAngle > MAX_PITCH_ANGLE)
	{
		mPitchAngle = MAX_PITCH_ANGLE;
	}

	// Rotation matrices
	Matrix4 pitch = Matrix4::CreateRotationY(mPitchAngle);

	float yawAngle = mOwner->GetTransform().GetRotation();
	Matrix4 yaw = Matrix4::CreateRotationZ(yawAngle);

	// Combined pitch + yaw
	Matrix4 combo = pitch * yaw;

	// Forward direction
	mCameraForward = Vector3::Transform(Vector3::UnitX, combo);

	// Eye position
	const Vector3 EYE = mOwner->GetTransform().GetPosition();

	// Target point ahead of camera
	Vector3 target = EYE + mCameraForward * CAMERA_FORWARD_DISTANCE;

	// Build view matrix
	Matrix4 view = Matrix4::CreateLookAt(EYE, target, Vector3::UnitZ);
	gGame.GetRenderer()->SetViewMatrix(view);
}
