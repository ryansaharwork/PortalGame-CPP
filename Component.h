//
// Created by Sanjay on 7/19/2025.
//

#pragma once
#include "SDL3/SDL_mouse.h"
#include "Math.h"

// Forward declaration (avoids circular includes)
class Actor;

class Component
{
public:
	// Getter for owner
	Actor* GetOwner() const;

	// Update entry point
	void Update(float deltaTime);

	// Input entry point
	void Input(const bool keys[], SDL_MouseButtonFlags mouseButtons, const Vector2& relativeMouse);

private:
	// Friendship - allow this class to use protected elements
	friend class Actor;

protected:
	Actor* mOwner = nullptr;

	// Explicit constructor
	explicit Component(Actor* owner);

	// Constructor
	Component() = default;

	// Deconstructor
	virtual ~Component();

	// Internal update func
	virtual void HandleUpdate(float deltaTime);

	// Internal input func
	virtual void HandleInput(const bool keys[], SDL_MouseButtonFlags mouseButtons,
							 const Vector2& relativeMouse);
};
