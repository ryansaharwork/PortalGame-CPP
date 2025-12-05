//
// Created by Ryan Sahar on 9/8/25.
//
#include "MoveComponent.h"
#include "Actor.h"

MoveComponent::MoveComponent(Actor* owner)
: Component(owner)
{
}

float MoveComponent::GetForwardSpeed() const
{
	return mForwardSpeed;
}

void MoveComponent::SetForwardSpeed(float speed)
{
	mForwardSpeed = speed;
}
float MoveComponent::GetAngularSpeed() const
{
	return mAngularSpeed;
}

void MoveComponent::SetAngularSpeed(float speed)
{
	mAngularSpeed = speed;
}

float MoveComponent::GetStrafeSpeed() const
{
	return mStrafeSpeed;
}

void MoveComponent::SetStrafeSpeed(float speed)
{
	mStrafeSpeed = speed;
}

void MoveComponent::HandleUpdate(float deltaTime)
{
	// Compute velocity = forward vector * speed
	const Vector3 FORWARD = mOwner->GetTransform().GetForward();
	const Vector3 RIGHT = mOwner->GetTransform().GetRight();
	const Vector3 VELOCITY = FORWARD * mForwardSpeed + RIGHT * mStrafeSpeed;

	// New position = old position + velocity * deltaTime
	Vector3 pos = mOwner->GetTransform().GetPosition();
	pos = pos + VELOCITY * deltaTime;

	// Updating the new position
	mOwner->GetTransform().SetPosition(pos);

	// Update rotation = old rotation + angular speed * deltaTime
	float rotation = mOwner->GetTransform().GetRotation();
	rotation = rotation + mAngularSpeed * deltaTime;
	mOwner->GetTransform().SetRotation(rotation);
}