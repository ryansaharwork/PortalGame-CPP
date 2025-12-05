//
// Created by Sanjay on 7/19/2025.
//

#include "Actor.h"
#include "Component.h"
#include "Game.h"

// This allows you to modify
Transform& Actor::GetTransform()
{
	return mTransform;
}

// This doesn't allow you to modify (good for read ins)
const Transform& Actor::GetTransform() const
{
	return mTransform;
}

void Actor::Update(float deltaTime)
{
	// If actor is false, do nothing
	if (!mIsActive)
	{
		return;
	}

	// Update all components
	for (Component* component : mComponents)
	{
		component->Update(deltaTime);
	}

	// Call actor specific update
	HandleUpdate(deltaTime);

	// Update all children
	for (Actor* child : mTransform.GetChildren())
	{
		if (child)
		{
			child->Update(deltaTime);
		}
	}
}

Actor::~Actor()
{
	gGame.GetAudio()->RemoveActor(this);

	// Delete all children
	for (Actor* child : mTransform.GetChildren())
	{
		delete child;
	}

	// Loops over mComponents vectors, deletes components
	for (const auto* c : mComponents)
	{
		delete c;
	}
	mComponents.clear();
}

void Actor::HandleUpdate(float /*deltaTime*/)
{
	// Leave empty for now
}

void Actor::Input(const bool keys[], SDL_MouseButtonFlags mouseButtons,
				  const Vector2& relativeMouse)
{
	// If actor isn't active, do nothing
	if (!mIsActive)
	{
		return;
	}

	// Update all components
	for (Component* component : mComponents)
	{
		component->Input(keys, mouseButtons, relativeMouse);
	}

	HandleInput(keys, mouseButtons, relativeMouse);

	// Input for all children
	for (Actor* child : mTransform.GetChildren())
	{
		if (child)
		{
			child->Input(keys, mouseButtons, relativeMouse);
		}
	}
}

bool Actor::IsActive() const
{
	return mIsActive;
}

void Actor::SetIsActive(bool active)
{
	mIsActive = active;
}

void Actor::Destroy()
{
	mIsActive = false;
	gGame.AddPendingDestroy(this);
}

void Actor::HandleInput(const bool keys[], SDL_MouseButtonFlags /*mouseButtons*/,
						const Vector2& /*relativeMouse*/)
{
	// Leave empty for now
}
