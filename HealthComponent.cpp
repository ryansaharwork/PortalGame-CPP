//
// Created by Ryan Sahar on 12/1/25.
//
#include "HealthComponent.h"

HealthComponent::HealthComponent(class Actor* owner)
: Component(owner)
{
}

void HealthComponent::TakeDamage(float damage, const Vector3& location)
{
	// Don't do anything if already dead
	if (IsDead())
	{
		return;
	}

	// Subtract the damage amount from the health
	mHealth -= damage;

	// Call the mOnDamage callback if it's set (passing in the location as a parameter)
	if (mOnDamage)
	{
		mOnDamage(location);
	}

	// If dead, call the mOnDeath callback if it's set
	if (IsDead() && mOnDeath)
	{
		mOnDeath();
	}
}