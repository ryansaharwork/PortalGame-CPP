#pragma once

#include "Actor.h"
#include "Math.h" // For Quaternion, Vector3, etc.
#include "AudioSystem.h"
#include <unordered_map>
#include <string>

class LaserComponent;
class HealthComponent;

// ----------------------
// Turret states
// ----------------------
enum class TurretState
{
	Idle,
	Search,
	Priming,
	Firing,
	Falling,
	Dead
};

class TurretHead : public Actor
{
public:
	LaserComponent* GetLaserComponent() const { return mLaserComp; }
	void Die();
	void TakeDamage();

protected:
	TurretHead();
	~TurretHead() override;
	friend class Game;
	friend class Actor;

	void HandleUpdate(float deltaTime) override;

private:
	// --- Existing laser members ---
	Actor* mLaserActor = nullptr;
	LaserComponent* mLaserComp = nullptr;

	// --- State machine members ---
	TurretState mState = TurretState::Idle;
	float mStateTimer = 0.0f;
	float mFiringCooldown = 0.0f;

	// --- Target acquisition ---
	class Actor* mAcquiredTarget = nullptr;

	// --- Portal teleporting ---
	Vector3 mFallVelocity = Vector3::Zero;
	float mPortalTeleportCooldown = 0.0f;
	bool CheckPortalTeleport();

	// --- Search motion members ---
	// Interpolation from mSearchStartQuat -> mSearchEndQuat over 0.5 seconds
	Quaternion mSearchStartQuat = Quaternion::Identity;
	Quaternion mSearchEndQuat = Quaternion::Identity;
	float mSearchInterpTime = 0.0f;
	bool mSearchGoingOut = true;   // true: center->target, false: target->center
	bool mHasSearchTarget = false; // whether we already chose a target rotation

	// --- State update functions ---
	void UpdateIdle(float deltaTime);
	void UpdateSearch(float deltaTime);
	void UpdatePriming(float deltaTime);
	void UpdateFiring(float deltaTime);
	void UpdateFalling(float deltaTime);
	void UpdateDead(float deltaTime);

	// --- Helper to change states ---
	void ChangeState(TurretState newState);

	// --- Target acquisition hook (stub for now) ---
	bool CheckTargetAcquisition();

	// --- Sound effects ---
	std::unordered_map<TurretState, std::string> mStateSounds;
	SoundHandle mCurrentVOSound;
	bool mHasTakenDamage = false;
};
