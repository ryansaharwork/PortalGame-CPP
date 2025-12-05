//
// Created by Ryan Sahar on 10/29/25.
//
#pragma once
#include "Component.h"
#include "Math.h"

class CameraComponent : public Component
{
public:
	float GetPitchAngle() const { return mPitchAngle; }
	float GetPitchSpeed() const { return mPitchSpeed; }

	void SetPitchSpeed(float speed) { mPitchSpeed = speed; }

	const Vector3& GetCameraForward() const { return mCameraForward; }

	void ResetPitch() { mPitchAngle = 0.0f; }

protected:
	CameraComponent(class Actor* owner);
	friend class Actor;
	void HandleUpdate(float deltaTime) override;

private:
	// ==== Tunable constants (no magic numbers) ====
	static constexpr float MAX_PITCH_ANGLE = Math::Pi / 2.1f; // Camera clamp
	static constexpr float CAMERA_FORWARD_DISTANCE = 50.0f;	  // Target distance

	// Pitch and rotation state
	float mPitchAngle = 0.0f;
	float mPitchSpeed = 0.0f;

	// Camera facing direction
	Vector3 mCameraForward = Vector3::UnitX;
};
