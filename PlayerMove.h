//
// Created by Ryan Sahar on 10/21/25.
//
#pragma once
#include "MoveComponent.h"
#include "CollisionComponent.h"
#include "AudioSystem.h"

class Crosshair;
class Portal;

class PlayerMove : public MoveComponent
{
public:
	// State enum
	enum class MoveState
	{
		OnGround,
		Jump,
		Falling
	};

	void ResetMove();
	const Vector3& GetVelocity() const { return mVelocity; }
	const Vector3& GetAcceleration() const { return mAcceleration; }

protected:
	PlayerMove(class Actor* owner);
	~PlayerMove() override;
	void HandleUpdate(float deltaTime) override;
	void HandleInput(const bool keys[], SDL_MouseButtonFlags mouseButtons,
					 const Vector2& relativeMouse) override;
	friend class Actor;

private:
	// ==== Tunable gameplay constants (no magic numbers) ====
	// Movement forces
	static constexpr float MOVE_FORCE_MAG = 700.0f;
	static constexpr float MAX_XY_SPEED = 400.0f;
	static constexpr float MIN_Z = -1000.0f;

	// Mouse sensitivity
	static constexpr float MOUSE_DIVISOR = 500.0f;
	static constexpr float MOUSE_TURN_MULTIPLIER = 10.0f;

	// Braking while on ground
	static constexpr float GROUND_BRAKE_FACTOR = 0.9f;

	// Portal ray distance
	static constexpr float PORTAL_MAX_DISTANCE = 1000.0f;

	// Portal teleport constants
	static constexpr float PORTAL_TELEPORT_COOLDOWN = 0.2f;
	static constexpr float PORTAL_TELEPORT_OFFSET = 50.0f;
	static constexpr float PORTAL_VELOCITY_MULTIPLIER = 1.5f;
	static constexpr float PORTAL_MIN_VELOCITY = 350.0f;

	// Player death threshold
	static constexpr float PLAYER_DEATH_Z = -750.0f;

	// Physics constants
	static constexpr Vector3 GRAVITY{0.0f, 0.0f, -980.0f};
	static constexpr Vector3 JUMP_FORCE{0.0f, 0.0f, 35000.0f};

	// Current state
	MoveState mCurrentState;

	// State change
	void ChangeState(MoveState state);

	// State update functions
	void UpdateOnGround(float deltaTime);
	void UpdateJump(float deltaTime);
	void UpdateFalling(float deltaTime);

	// Physics variables
	Vector3 mVelocity;
	Vector3 mAcceleration;
	Vector3 mPendingForces;
	float mMass = 1.0f;
	bool mPrevSpace = false;
	bool mIsTeleportFalling = false;

	// Physics helpers
	void PhysicsUpdate(float deltaTime);
	void AddForce(const Vector3& force);
	void FixXYVelocity();
	CollSide FixCollision(const CollisionComponent* self, const CollisionComponent* collider) const;

	Crosshair* mCrosshair = nullptr;

	bool mPrevLeftMouse = false;
	bool mPrevRightMouse = false;
	bool mPrevR = false;

	void CreatePortal(bool isBlue) const;

	// Portal teleport helpers
	bool UpdatePortalTeleport(float deltaTime);
	void DoPortalTeleport(class Portal* entryPortal, class Portal* exitPortal);
	float mPortalTeleportCooldown = 0.0f;

	// Sound effects
	SoundHandle mFootstepSound;
	MoveState mPrevState = MoveState::OnGround;
};
