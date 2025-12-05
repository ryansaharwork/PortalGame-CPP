//
// Created by Sanjay on 7/19/2025.
//

#include "Component.h"

// Constructor definition
// Assigns the Actor* to mOwner
Component::Component(Actor* owner)
{
	mOwner = owner;
}

// Destructor definition
Component::~Component() = default;

// GetOwner() definition
// Returns the owner
Actor* Component::GetOwner() const
{
	return mOwner;
}
void Component::Update(float deltaTime)
{
	HandleUpdate(deltaTime);
}

void Component::Input(const bool keys[], SDL_MouseButtonFlags mouseButtons,
					  const Vector2& relativeMouse)
{
	HandleInput(keys, mouseButtons, relativeMouse);
}

void Component::HandleUpdate(float deltaTime)
{
	// Leave empty for now
}

void Component::HandleInput(const bool keys[], SDL_MouseButtonFlags /*mouseButtons*/,
							const Vector2& /*relativeMouse*/)
{
	// Leave empty for now
}
