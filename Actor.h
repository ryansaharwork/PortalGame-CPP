//
// Created by Sanjay on 7/19/2025.
//
#pragma once

#include "Transform.h"
#include "Math.h"
#include "SDL3/SDL_mouse.h"
#include <vector>

// Forward declaration (avoids circular includes)
class Component;

class Actor
{
public:
	// This version allows us to modify the transform
	Transform& GetTransform();

	// This version is for cases where we don't need to modify
	const Transform& GetTransform() const;

	// templated CreateChild()
	template <typename T>
	T* CreateChild()
	{
		T* child = new T();
		child->GetTransform().SetupParent(child, this);
		return child;
	}

	// templated CreateComponent()
	template <typename T>
	T* CreateComponent()
	{
		// Create a new instance of type T, which should inherit from Component
		// It takes in "this" because every component takes an Actor* in its constructor
		T* component = new T(this);

		// Add this component to our component vector
		mComponents.emplace_back(component);
		return component;
	}

	// Returns component of type T, or nullptr if it doesn't exist
	template <typename T>
	T* GetComponent() const
	{
		// Loop over all components
		for (auto c : mComponents)
		{
			// dynamic_cast will return nullptr if c is not type T*
			T* t = dynamic_cast<T*>(c);
			if (t)
			{
				return t;
			}
		}
		return nullptr;
	}

	void Update(float deltaTime);
	void Input(const bool keys[], SDL_MouseButtonFlags mouseButtons, const Vector2& relativeMouse);

	// Getter and setter for active boolean
	bool IsActive() const;
	void SetIsActive(bool active);

	// Destroys an actor (sets actor to false)
	void Destroy();

private:
	// Bool for if the actor is active or not
	bool mIsActive = true;

	// Vector holding all the components
	std::vector<Component*> mComponents;

	// Friendship - allow this class to use protected elements
	friend class Game;

protected:
	// Tracks actor's components
	Transform mTransform;

	// Default constructor
	Actor() = default;

	// Virtual destructor that deletes owned components
	virtual ~Actor();

	virtual void HandleUpdate(float deltaTime);
	virtual void HandleInput(const bool keys[], SDL_MouseButtonFlags mouseButtons,
							 const Vector2& relativeMouse);
};
