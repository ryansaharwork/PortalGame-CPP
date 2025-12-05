//
// Created by Ryan Sahar on 12/1/25.
//
#pragma once
#include "Component.h"
#include "Math.h"
#include <functional>

class HealthComponent : public Component
{
public:
	float GetHealth() const { return mHealth; }
	bool IsDead() const { return mHealth <= 0.0f; }
	void TakeDamage(float damage, const Vector3& location);

	// Setter for both callbacks
	void SetOnDamage(const std::function<void(const Vector3&)>& onDamage) { mOnDamage = onDamage; }
	void SetOnDeath(const std::function<void()>& onDeath) { mOnDeath = onDeath; }

protected:
	HealthComponent(class Actor* owner);
	friend class Actor;

private:
	float mHealth = 100.0f;
	std::function<void(const Vector3&)> mOnDamage;
	std::function<void()> mOnDeath;
};