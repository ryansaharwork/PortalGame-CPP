//
// Created by Ryan Sahar on 9/8/25.
//
#pragma once
#include "Component.h"

class Actor;

class MoveComponent : public Component
{
public:
	// Getter/Setter for forward speed
	float GetForwardSpeed() const;
	void SetForwardSpeed(float speed);

	// Getter/Setter for angular speed
	float GetAngularSpeed() const;
	void SetAngularSpeed(float speed);

	// Getter/Setter for strafe speed
	float GetStrafeSpeed() const;
	void SetStrafeSpeed(float speed);

private:
	float mForwardSpeed = 0.0f;
	float mAngularSpeed = 0.0f;
	float mStrafeSpeed = 0.0f;

protected:
	explicit MoveComponent(Actor* owner);
	void HandleUpdate(float deltaTime) override;

	friend class Actor;
};