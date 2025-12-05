#include "PlayerMove.h"

#include "CameraComponent.h"
#include "Actor.h"
#include "Block.h"

#include <SDL3/SDL_scancode.h>
#include <cmath>
#include "Game.h"
#include "Crosshair.h"
#include "Player.h"
#include "Renderer.h"
#include "SegmentCast.h"
#include "Portal.h"
#include "HealthComponent.h"
#include "Math.h"

void PlayerMove::ResetMove()
{
	// Zero out motion
	mVelocity = Vector3::Zero;
	mAcceleration = Vector3::Zero;
	mPendingForces = Vector3::Zero;

	// Stop angular rotation
	SetAngularSpeed(0.0f);

	// Clear input states
	mPrevSpace = false;
	mPrevLeftMouse = false;
	mPrevRightMouse = false;
	mPrevR = false;

	ChangeState(MoveState::OnGround);
}

PlayerMove::PlayerMove(class Actor* owner)
: MoveComponent(owner)
, mCurrentState(MoveState::OnGround)
, mPrevState(MoveState::OnGround)
{
	// Create crosshair for player
	mCrosshair = mOwner->CreateComponent<Crosshair>();

	// Start footstep sound (looping) and immediately pause it
	mFootstepSound = gGame.GetAudio()->PlaySound("FootstepLoop.ogg", true);
	gGame.GetAudio()->PauseSound(mFootstepSound);
}

PlayerMove::~PlayerMove()
{
	gGame.GetAudio()->StopSound(mFootstepSound);
}

void PlayerMove::HandleUpdate(float deltaTime)
{
	// Check if player is dead
	HealthComponent* health = mOwner->GetComponent<HealthComponent>();
	if (health && health->IsDead())
	{
		Player* player = dynamic_cast<Player*>(mOwner);
		if (player)
		{
			SoundHandle deathSound = player->GetDeathSound();
			if (deathSound.IsValid() &&
				gGame.GetAudio()->GetSoundState(deathSound) == SoundState::Stopped)
			{
				gGame.SetNextLevel(gGame.GetCurrentLevel());
			}
		}
		return;
	}

	// Check if player fell too far (but not during replay playback)
	Vector3 pos = mOwner->GetTransform().GetPosition();
	if (pos.z <= PLAYER_DEATH_Z && !gGame.GetInputReplay()->IsInPlayback())
	{
		// Instead of directly reloading the level, the player's health component should take damage
		if (health)
		{
			health->TakeDamage(Math::Infinity, mOwner->GetTransform().GetPosition());
		}
	}

	// Footstep sound logic
	if (mCurrentState == MoveState::OnGround && mVelocity.Length() > 50.0f)
	{
		if (gGame.GetAudio()->GetSoundState(mFootstepSound) == SoundState::Paused)
		{
			gGame.GetAudio()->ResumeSound(mFootstepSound);
		}
	}
	else
	{
		if (gGame.GetAudio()->GetSoundState(mFootstepSound) == SoundState::Playing)
		{
			gGame.GetAudio()->PauseSound(mFootstepSound);
		}
	}

	// Check for landing sound (state change from Falling to OnGround)
	if (mPrevState == MoveState::Falling && mCurrentState == MoveState::OnGround)
	{
		gGame.GetAudio()->PlaySound("Land.ogg");
	}

	mPrevState = mCurrentState;

	switch (mCurrentState)
	{
	case MoveState::Falling:
		UpdateFalling(deltaTime);
		break;
	case MoveState::Jump:
		UpdateJump(deltaTime);
		break;
	case MoveState::OnGround:
		UpdateOnGround(deltaTime);
		break;
	}
}

void PlayerMove::HandleInput(const bool keys[], SDL_MouseButtonFlags mouseButtons,
							 const Vector2& relativeMouse)
{
	// Don't do anything if player is dead
	HealthComponent* health = mOwner->GetComponent<HealthComponent>();
	if (health && health->IsDead())
	{
		return;
	}

	// FORWARD/BACKWARD FORCE
	const bool W = keys[SDL_SCANCODE_W];
	const bool S = keys[SDL_SCANCODE_S];

	if (W)
	{
		AddForce(mOwner->GetTransform().GetForward() * MOVE_FORCE_MAG);
	}

	if (S)
	{
		AddForce(mOwner->GetTransform().GetForward() * -MOVE_FORCE_MAG);
	}

	// STRAFE FORCE
	const bool D = keys[SDL_SCANCODE_D];
	const bool A = keys[SDL_SCANCODE_A];

	if (D)
	{
		AddForce(mOwner->GetTransform().GetRight() * MOVE_FORCE_MAG);
	}

	if (A)
	{
		AddForce(mOwner->GetTransform().GetRight() * -MOVE_FORCE_MAG);
	}

	// CAMERA
	// Mouse yaw (turning w/ mouse)
	float angularSpeed = (relativeMouse.x / MOUSE_DIVISOR) * Math::Pi * MOUSE_TURN_MULTIPLIER;
	SetAngularSpeed(angularSpeed);

	// Pitch speed uses mouse Y
	float pitchSpeed = (relativeMouse.y / MOUSE_DIVISOR) * Math::Pi * MOUSE_TURN_MULTIPLIER;
	if (CameraComponent* cam = mOwner->GetComponent<CameraComponent>())
	{
		cam->SetPitchSpeed(pitchSpeed);
	}

	// JUMP
	const bool SPACE_NOW = keys[SDL_SCANCODE_SPACE];
	if (SPACE_NOW && !mPrevSpace && mCurrentState == MoveState::OnGround)
	{
		AddForce(JUMP_FORCE);
		ChangeState(MoveState::Jump);
		gGame.GetAudio()->PlaySound("Jump.ogg");
	}
	mPrevSpace = SPACE_NOW;

	// MOUSE PORTAL DETECTION
	const bool LEFT_NOW = (mouseButtons & SDL_BUTTON_MASK(SDL_BUTTON_LEFT)) != 0;
	const bool RIGHT_NOW = (mouseButtons & SDL_BUTTON_MASK(SDL_BUTTON_RIGHT)) != 0;

	if (Player* p = gGame.GetPlayer(); p && p->HasGun())
	{
		// Left click: BLUE portal
		if (LEFT_NOW && !mPrevLeftMouse)
		{
			CreatePortal(true);
		}

		// Right click: ORANGE portal
		if (RIGHT_NOW && !mPrevRightMouse)
		{
			CreatePortal(false);
		}
	}

	// Store mouse button states
	mPrevLeftMouse = LEFT_NOW;
	mPrevRightMouse = RIGHT_NOW;

	// RESET PORTALS
	const bool R_NOW = keys[SDL_SCANCODE_R];

	if (R_NOW && !mPrevR)
	{
		Portal* bluePortal = gGame.GetBluePortal();
		Portal* orangePortal = gGame.GetOrangePortal();

		if (bluePortal || orangePortal)
		{
			gGame.GetAudio()->PlaySound("PortalClose.ogg");
		}

		if (bluePortal)
		{
			gGame.AddPendingDestroy(bluePortal);
			gGame.SetBluePortal(nullptr);
		}

		if (orangePortal)
		{
			gGame.AddPendingDestroy(orangePortal);
			gGame.SetOrangePortal(nullptr);
		}

		mCrosshair->SetState(CrosshairState::Default);
	}

	mPrevR = R_NOW;
}

void PlayerMove::ChangeState(MoveState state)
{
	mCurrentState = state;
}

void PlayerMove::UpdateOnGround(float deltaTime)
{
	PhysicsUpdate(deltaTime);

	// Check if portal teleport before any collision checks
	if (UpdatePortalTeleport(deltaTime))
	{
		return;
	}

	// Check collisions and if we're standing on anything
	bool onTop = false;

	const CollisionComponent* self = mOwner->GetComponent<CollisionComponent>();
	if (self)
	{
		for (Actor* actor : gGame.GetColliders())
		{
			const CollisionComponent* other = actor->GetComponent<CollisionComponent>();
			if (other)
			{
				CollSide side = FixCollision(self, other);
				if (side == CollSide::Top)
				{
					onTop = true;
				}
			}
		}
	}

	// If we weren't on top of any collisions, start falling
	if (!onTop)
	{
		ChangeState(MoveState::Falling);
	}
}

void PlayerMove::UpdateJump(float deltaTime)
{
	AddForce(GRAVITY);
	PhysicsUpdate(deltaTime);

	// Check for portal teleport before any collision checks
	if (UpdatePortalTeleport(deltaTime))
	{
		return;
	}

	const CollisionComponent* self = mOwner->GetComponent<CollisionComponent>();
	if (self)
	{
		for (Actor* actor : gGame.GetColliders())
		{
			const CollisionComponent* other = actor->GetComponent<CollisionComponent>();
			if (other)
			{
				CollSide side = FixCollision(self, other);
				if (side == CollSide::Bottom)
				{
					mVelocity.z = 0.0f;
				}
			}
		}
	}

	// Reached max jump (you're now falling)
	if (mVelocity.z <= 0.0f)
	{
		ChangeState(MoveState::Falling);
	}
}

void PlayerMove::UpdateFalling(float deltaTime)
{
	AddForce(GRAVITY); // Add gravity
	PhysicsUpdate(deltaTime);

	// Check for portal teleport before any collision checks
	if (UpdatePortalTeleport(deltaTime))
	{
		return;
	}

	bool landed = false;

	const CollisionComponent* self = mOwner->GetComponent<CollisionComponent>();
	if (self)
	{
		for (Actor* actor : gGame.GetColliders())
		{
			const CollisionComponent* other = actor->GetComponent<CollisionComponent>();
			if (other)
			{
				CollSide side = FixCollision(self, other);
				if (side == CollSide::Top && mVelocity.z <= 0.0f)
				{
					landed = true;
				}
			}
		}
	}

	if (landed)
	{
		mVelocity.z = 0.0f;
		ChangeState(MoveState::OnGround);

		// No longer teleport falling
		mIsTeleportFalling = false;
	}
}

void PlayerMove::PhysicsUpdate(float deltaTime)
{
	// F = m * a -> a = F / m
	mAcceleration = mPendingForces * (1.0f / mMass);

	// Update velocity
	mVelocity += mAcceleration * deltaTime;

	// Fix XY velocity limits/braking
	FixXYVelocity();

	// Terminal Z velocity cap
	if (mVelocity.z <= MIN_Z)
	{
		mVelocity.z = MIN_Z;
	}

	// Update position
	Vector3 pos = mOwner->GetTransform().GetPosition();
	pos += mVelocity * deltaTime;
	mOwner->GetTransform().SetPosition(pos);

	// Update rotation
	float rotation = mOwner->GetTransform().GetRotation();
	rotation += GetAngularSpeed() * deltaTime;
	mOwner->GetTransform().SetRotation(rotation);

	// Reset forces
	mPendingForces = Vector3::Zero;
}

void PlayerMove::AddForce(const Vector3& force)
{
	mPendingForces += force;
}

void PlayerMove::FixXYVelocity()
{
	Vector2 xyVelocity(mVelocity.x, mVelocity.y);

	// Limit XY speed (unless you're falling due to portal teleport)
	if (!mIsTeleportFalling)
	{
		if (xyVelocity.Length() > MAX_XY_SPEED)
		{
			xyVelocity.Normalize();
			xyVelocity *= MAX_XY_SPEED;
		}
	}

	// If on ground, apply braking
	if (mCurrentState == MoveState::OnGround)
	{
		// Braking for X
		if (Math::NearlyZero(mAcceleration.x) || (mAcceleration.x > 0.0f && xyVelocity.x < 0.0f) ||
			(mAcceleration.x < 0.0f && xyVelocity.x > 0.0f))
		{
			xyVelocity.x *= GROUND_BRAKE_FACTOR;
		}

		// Braking for Y
		if (Math::NearlyZero(mAcceleration.y) || (mAcceleration.y > 0.0f && xyVelocity.y < 0.0f) ||
			(mAcceleration.y < 0.0f && xyVelocity.y > 0.0f))
		{
			xyVelocity.y *= GROUND_BRAKE_FACTOR;
		}
	}

	// Update velocity w/ fixed XY values
	mVelocity.x = xyVelocity.x;
	mVelocity.y = xyVelocity.y;
}

CollSide PlayerMove::FixCollision(const CollisionComponent* self,
								  const CollisionComponent* collider) const
{
	Vector3 offset;
	CollSide side = self->GetMinOverlap(collider, offset);
	if (side != CollSide::None)
	{
		Vector3 pos = mOwner->GetTransform().GetPosition();
		pos += offset;
		mOwner->GetTransform().SetPosition(pos);
	}
	return side;
}

void PlayerMove::CreatePortal(bool isBlue) const
{
	// Create a line segment from center of screen
	Renderer* renderer = gGame.GetRenderer();

	constexpr Vector3 SCREEN_CENTER_NEAR(0.0f, 0.0f, 0.0f);
	constexpr Vector3 SCREEN_CENTER_FAR(0.0f, 0.0f, 1.0f);

	const Vector3 NEAR_POINT = renderer->Unproject(SCREEN_CENTER_NEAR);
	const Vector3 FAR_POINT = renderer->Unproject(SCREEN_CENTER_FAR);

	Vector3 dir = FAR_POINT - NEAR_POINT;
	dir.Normalize();

	LineSegment segment;
	segment.mStart = NEAR_POINT;
	segment.mEnd = NEAR_POINT + dir * PORTAL_MAX_DISTANCE;

	// Cache current portals
	Portal* bluePortal = gGame.GetBluePortal();
	Portal* orangePortal = gGame.GetOrangePortal();

	// Segment cast
	CastInfo cast;
	if (SegmentCast(gGame.GetColliders(), segment, cast))
	{
		// Allow portal to attach to blocks
		Block* hitBlock = dynamic_cast<Block*>(cast.mActor);
		if (!hitBlock)
		{
			gGame.GetAudio()->PlaySound("PortalFail.ogg");
			return; // Only blocks can hold a portal
		}

		// If it's a block, create a new portal
		Portal* portal = gGame.CreateActor<Portal>();
		portal->Setup(cast.mPoint, cast.mNormal, isBlue);

		// Play appropriate sound
		if (isBlue)
		{
			gGame.GetAudio()->PlaySound("PortalShootBlue.ogg");
		}
		else
		{
			gGame.GetAudio()->PlaySound("PortalShootOrange.ogg");
		}

		// Destroy existing portal of the same color and update locals
		if (isBlue)
		{
			if (bluePortal)
			{
				gGame.AddPendingDestroy(bluePortal);
			}
			gGame.SetBluePortal(portal);
			bluePortal = portal;
		}
		else
		{
			if (orangePortal)
			{
				gGame.AddPendingDestroy(orangePortal);
			}
			gGame.SetOrangePortal(portal);
			orangePortal = portal;
		}
	}
	else
	{
		gGame.GetAudio()->PlaySound("PortalFail.ogg");
	}

	// UPDATE CROSSHAIR using cached portals
	if (bluePortal && orangePortal)
	{
		mCrosshair->SetState(CrosshairState::BothFill);
	}
	else if (bluePortal)
	{
		mCrosshair->SetState(CrosshairState::BlueFill);
	}
	else if (orangePortal)
	{
		mCrosshair->SetState(CrosshairState::OrangeFill);
	}
	else
	{
		mCrosshair->SetState(CrosshairState::Default);
	}
}

bool PlayerMove::UpdatePortalTeleport(float deltaTime)
{
	// Handle cooldown so we don't instantly re-teleport
	if (mPortalTeleportCooldown > 0.0f)
	{
		mPortalTeleportCooldown -= deltaTime;
		if (mPortalTeleportCooldown > 0.0f)
		{
			return false;
		}
	}

	Portal* bluePortal = gGame.GetBluePortal();
	Portal* orangePortal = gGame.GetOrangePortal();

	// Need both portals to teleport
	if (!bluePortal || !orangePortal)
	{
		return false;
	}

	const CollisionComponent* self = mOwner->GetComponent<CollisionComponent>();
	if (!self)
	{
		return false;
	}

	Portal* entryPortal = nullptr;
	Portal* exitPortal = nullptr;

	// Check intersection with blue portal
	if (CollisionComponent* blueColl = bluePortal->GetComponent<CollisionComponent>();
		blueColl && self->Intersect(blueColl))
	{
		entryPortal = bluePortal;
		exitPortal = orangePortal;
	}
	// Check intersection with orange portal
	else if (CollisionComponent* orangeColl = orangePortal->GetComponent<CollisionComponent>();
			 orangeColl && self->Intersect(orangeColl))
	{
		entryPortal = orangePortal;
		exitPortal = bluePortal;
	}

	if (!entryPortal)
	{
		return false;
	}

	// Perform the teleport
	DoPortalTeleport(entryPortal, exitPortal);
	gGame.GetAudio()->PlaySound("PortalTeleport.ogg");

	// Start cooldown
	mPortalTeleportCooldown = PORTAL_TELEPORT_COOLDOWN;

	return true;
}

void PlayerMove::DoPortalTeleport(Portal* entryPortal, Portal* exitPortal)
{
	if (!entryPortal || !exitPortal)
	{
		return;
	}

	Transform& xform = mOwner->GetTransform();

	// NEW PLAYER POSITION

	Vector3 entryForward = entryPortal->GetTransform().GetForward();
	Vector3 exitForward = exitPortal->GetTransform().GetForward();

	Vector3 baseTeleportPos;

	// If entry OR exit is along Z, just use exit portal position
	if (Math::NearlyEqual(fabsf(entryForward.z), 1.0f) ||
		Math::NearlyEqual(fabsf(exitForward.z), 1.0f))
	{
		baseTeleportPos = exitPortal->GetTransform().GetPosition();
	}

	// Otherwise, transform current player position through portals (w = 1)
	else
	{
		Vector3 pos = xform.GetPosition();
		baseTeleportPos = entryPortal->GetPortalOutVector(pos, exitPortal, 1.0f);
	}

	// Offset along exit portal forward so we don't clip into wall
	Vector3 newPos = baseTeleportPos + exitForward * PORTAL_TELEPORT_OFFSET;
	xform.SetPosition(newPos);

	// NEW VELOCITY
	// Calculate new velocity length: max(multiplier * current_length, min_velocity)
	float currentVelLength = mVelocity.Length();
	float newVelLength = Math::Max(PORTAL_VELOCITY_MULTIPLIER * currentVelLength,
								   PORTAL_MIN_VELOCITY);

	// Determine new velocity direction
	Vector3 newVelDirection;
	// If entry OR exit is along +/- Z, use exit portal's forward
	if (Math::NearlyEqual(fabsf(entryForward.z), 1.0f) ||
		Math::NearlyEqual(fabsf(exitForward.z), 1.0f))
	{
		newVelDirection = exitForward;
	}
	// Otherwise, transform original velocity direction through portals (w = 0)
	else
	{
		Vector3 originalVelDir = mVelocity;
		originalVelDir.Normalize();
		newVelDirection = entryPortal->GetPortalOutVector(originalVelDir, exitPortal, 0.0f);
	}

	// Combine new direction and length
	newVelDirection.Normalize();
	mVelocity = newVelDirection * newVelLength;

	// NEW PLAYER ROTATION
	// If exit portal faces along Z, do NOT change rotation
	if (!Math::NearlyEqual(fabsf(exitForward.z), 1.0f))
	{
		// Figure out desired new facing
		Vector3 desiredFacing;

		// If entry is along Z, just use exit portal's forward
		if (Math::NearlyEqual(fabsf(entryForward.z), 1.0f))
		{
			desiredFacing = exitForward;
		}

		// Otherwise, transform original player forward through portals
		else
		{
			Vector3 oldForward = xform.GetForward();
			desiredFacing = entryPortal->GetPortalOutVector(oldForward, exitPortal, 0.0f);
		}

		// Work in the X/Y plane for yaw
		desiredFacing.z = 0.0f;
		desiredFacing.Normalize();

		// Angle between unit X and desiredFacing
		constexpr Vector3 BASE_FORWARD = Vector3::UnitX;
		float dot = Vector3::Dot(BASE_FORWARD, desiredFacing);
		dot = Math::Clamp(dot, -1.0f, 1.0f);
		float angle = Math::Acos(dot);

		Vector3 cross = Vector3::Cross(BASE_FORWARD, desiredFacing);
		if (cross.z < 0.0f)
		{
			angle = -angle;
		}

		xform.SetRotation(angle);
	}

	// After teleport, player should be in Falling state
	ChangeState(MoveState::Falling);

	// Mark that we are falling due to teleport
	mIsTeleportFalling = true;
}
