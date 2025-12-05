#include "TurretHead.h"
#include "MeshComponent.h"
#include "Game.h"
#include "Renderer.h"
#include "Math.h"
#include "LaserComponent.h"
#include "HealthComponent.h"
#include "Portal.h"
#include "CollisionComponent.h"
#include "TurretBase.h"
#include "Random.h" // For Random::GetFloatRange
#include "AudioSystem.h"

TurretHead::TurretHead()
{
	// Local offset above the base
	GetTransform().SetPosition(Vector3(0.0f, 0.0f, 25.0f));

	// Head mesh
	MeshComponent* mesh = CreateComponent<MeshComponent>();
	mesh->SetMesh(gGame.GetRenderer()->GetMesh("Assets/Meshes/TurretHead.gpmesh"));

	// LASER CHILD
	mLaserActor = CreateChild<Actor>();
	if (mLaserActor)
	{
		// Laser actor is slightly in front of the head
		mLaserActor->GetTransform().SetPosition(Vector3(0.0f, 0.0f, 13.0f));
		mLaserComp = mLaserActor->CreateComponent<LaserComponent>();
	}

	// Start in Idle by default
	mState = TurretState::Idle;
	mStateTimer = 0.0f;

	// Search-related defaults
	mSearchStartQuat = Quaternion::Identity;
	mSearchEndQuat = Quaternion::Identity;
	mSearchInterpTime = 0.0f;
	mSearchGoingOut = true;
	mHasSearchTarget = false;

	// Initialize state sounds map
	mStateSounds[TurretState::Idle] = "TurretIdle.ogg";
	mStateSounds[TurretState::Search] = "TurretSearch.ogg";
	mStateSounds[TurretState::Priming] = "TurretPriming.ogg";
	mStateSounds[TurretState::Firing] = "TurretFiring.ogg";
	mStateSounds[TurretState::Falling] = "TurretFalling.ogg";
	mStateSounds[TurretState::Dead] = "TurretDead.ogg";

	mCurrentVOSound = SoundHandle::Invalid;
}

TurretHead::~TurretHead() = default;

// ------------------------
// State update stubs
// (Idle/Priming/Firing/Falling/Dead will be filled later)
// ------------------------
void TurretHead::UpdateIdle(float /*deltaTime*/)
{
	// Check for portal teleport first
	if (CheckPortalTeleport())
	{
		ChangeState(TurretState::Falling);
		return;
	}

	// In the Idle state, if a target is acquired, switch to the Priming state
	if (CheckTargetAcquisition())
	{
		ChangeState(TurretState::Priming);
	}
}

void TurretHead::UpdateSearch(float deltaTime)
{
	// Check for portal teleport first
	if (CheckPortalTeleport())
	{
		ChangeState(TurretState::Falling);
		return;
	}

	// 1. If a target is acquired while searching, go to Priming.
	if (CheckTargetAcquisition())
	{
		ChangeState(TurretState::Priming);
		return;
	}

	// 2. If we don't yet have a search target rotation, choose a new one.
	if (!mHasSearchTarget)
	{
		// If more than 5 seconds have elapsed in the search state, return to Idle
		if (mStateTimer >= 5.0f)
		{
			ChangeState(TurretState::Idle);
			return;
		}

		// Constants from the instructions
		constexpr float SIDE_DIST = 75.0f;
		constexpr float UP_DIST = 25.0f;
		constexpr float FWD_DIST = 200.0f;

		// Basis vectors from the head
		const Vector3 HEAD_POS = GetTransform().GetPosition();
		const Vector3 FORWARD = GetTransform().GetForward();
		const Vector3 RIGHT = GetTransform().GetRight();
		constexpr Vector3 UP = Vector3::UnitZ; // world up

		// Center of the "search circle"
		Vector3 center = HEAD_POS + FORWARD * FWD_DIST;

		// Random angle around the circle
		float angle = Random::GetFloatRange(0.0f, Math::TwoPi);

		// Point on an ellipse defined by SideDist and UpDist
		Vector3 offset = RIGHT * (SIDE_DIST * Math::Cos(angle)) + UP * (UP_DIST * Math::Sin(angle));

		Vector3 targetPos = center + offset;

		// Direction we want to face (in local space where +X is "forward")
		Vector3 toTarget = Vector3::Normalize(targetPos - HEAD_POS);
		constexpr Vector3 ORIGINAL_FACING = Vector3::UnitX;

		float dot = Vector3::Dot(ORIGINAL_FACING, toTarget);
		Quaternion targetQuat;

		if (Math::NearlyEqual(dot, 1.0f))
		{
			targetQuat = Quaternion::Identity;
		}
		else if (Math::NearlyEqual(dot, -1.0f))
		{
			// 180 degrees around Z
			targetQuat = Quaternion(Vector3::UnitZ, Math::Pi);
		}
		else
		{
			Vector3 axis = Vector3::Normalize(Vector3::Cross(ORIGINAL_FACING, toTarget));
			float angleToTarget = Math::Acos(dot);
			targetQuat = Quaternion(axis, angleToTarget);
		}

		// Start at center (identity), first leg goes center -> target
		mSearchStartQuat = Quaternion::Identity;
		mSearchEndQuat = targetQuat;
		mSearchInterpTime = 0.0f;
		mSearchGoingOut = true;
		mHasSearchTarget = true;
	}

	// 3. Interpolate rotation over 0.5 seconds for the current leg.
	constexpr float HALF_CYCLE = 0.5f; // seconds for each leg (out or back)
	mSearchInterpTime += deltaTime;

	float t = mSearchInterpTime / HALF_CYCLE;
	if (t < 0.0f)
		t = 0.0f;
	if (t > 1.0f)
		t = 1.0f;

	Quaternion q = Quaternion::Slerp(mSearchStartQuat, mSearchEndQuat, t);
	GetTransform().SetQuat(q);

	// 4. If we finished this leg, set up the next one.
	if (mSearchInterpTime >= HALF_CYCLE)
	{
		// Keep any leftover time in case deltaTime > HalfCycle
		mSearchInterpTime -= HALF_CYCLE;

		if (mSearchGoingOut)
		{
			// We just finished center -> target. Now go target -> center.
			mSearchGoingOut = false;
			mSearchStartQuat = mSearchEndQuat;
			mSearchEndQuat = Quaternion::Identity;
		}
		else
		{
			// We just finished target -> center. Full 1-second cycle done.
			// Next update, pick a new random target.
			mSearchGoingOut = true;
			mHasSearchTarget = false;
			mSearchStartQuat = Quaternion::Identity;
			mSearchEndQuat = Quaternion::Identity;
		}
	}

	// IMPORTANT: We *do not* reset any of these search variables when the turret
	// leaves the Search state. That way, if we exit to another state and later
	// come back to Search, this motion can resume from wherever it was.
}

void TurretHead::UpdatePriming(float /*deltaTime*/)
{
	// Check for portal teleport first
	if (CheckPortalTeleport())
	{
		ChangeState(TurretState::Falling);
		return;
	}

	// Check every frame if the last hit actor is still the acquired target
	if (mLaserComp)
	{
		Actor* lastHit = mLaserComp->GetLastHitActor();
		if (lastHit != mAcquiredTarget)
		{
			// Target lost, switch to Search
			ChangeState(TurretState::Search);
			return;
		}
	}

	// If at least 1.5 seconds has elapsed in Priming, switch to Firing
	if (mStateTimer >= 1.5f)
	{
		ChangeState(TurretState::Firing);
	}
}

void TurretHead::UpdateFiring(float deltaTime)
{
	// Check for portal teleport first
	if (CheckPortalTeleport())
	{
		ChangeState(TurretState::Falling);
		return;
	}

	// In the Firing state, if the last hit actor is no longer the acquired target,
	// then switch to Search
	if (mLaserComp)
	{
		Actor* lastHit = mLaserComp->GetLastHitActor();
		if (lastHit != mAcquiredTarget)
		{
			ChangeState(TurretState::Search);
			return;
		}
	}

	// Update firing cooldown
	if (mFiringCooldown > 0.0f)
	{
		mFiringCooldown -= deltaTime;
		if (mFiringCooldown < 0.0f)
		{
			mFiringCooldown = 0.0f;
		}
	}

	// If the target is still valid (meaning the turret has not switched to the search state)
	// AND the target's health component does not indicate it is dead, the turret should deal damage
	if (mAcquiredTarget)
	{
		HealthComponent* targetHealth = mAcquiredTarget->GetComponent<HealthComponent>();
		if (targetHealth && !targetHealth->IsDead())
		{
			// Damage should be dealt every 0.05 seconds
			if (mFiringCooldown <= 0.0f)
			{
				// Each time the turret deals damage, it should deal 2.5 damage
				// The position where the damage is coming from should be the turret's own position
				Vector3 turretPos = GetTransform().GetPosition();
				targetHealth->TakeDamage(2.5f, turretPos);

				// Play bullet sound on the target
				gGame.GetAudio()->PlaySound("Bullet.ogg", false, mAcquiredTarget);

				// To prevent shooting too quickly, the cooldown should be reset to 0.05 every time the turret fires
				mFiringCooldown = 0.05f;
			}
		}
	}
}

void TurretHead::UpdateFalling(float deltaTime)
{
	// Get the parent (TurretBase)
	Actor* parent = GetTransform().GetParent();
	if (!parent)
	{
		return;
	}

	// 1. Update the parent's position according to the fall velocity and delta time
	Vector3 parentPos = parent->GetTransform().GetPosition();
	parentPos += mFallVelocity * deltaTime;
	parent->GetTransform().SetPosition(parentPos);

	// 2. Check for a portal teleport (using the helper function). If you don't teleport:
	if (!CheckPortalTeleport())
	{
		// 2a. Update velocity according to a gravity of <0, 0, -980> and delta time
		constexpr Vector3 GRAVITY(0.0f, 0.0f, -980.0f);
		mFallVelocity += GRAVITY * deltaTime;

		// 2b. Use GetMinOverlap between the parent's collision and all colliders
		// (keep in mind that the parent is also a collider, and you don't want to collide against that)
		CollisionComponent* parentColl = parent->GetComponent<CollisionComponent>();
		if (parentColl)
		{
			for (Actor* collider : gGame.GetColliders())
			{
				// Skip the parent itself
				if (collider == parent)
				{
					continue;
				}

				CollisionComponent* otherColl = collider->GetComponent<CollisionComponent>();
				if (!otherColl)
				{
					continue;
				}

				Vector3 offset;
				CollSide side = parentColl->GetMinOverlap(otherColl, offset);
				if (side != CollSide::None)
				{
					// Update position based on offset
					Vector3 currentPos = parent->GetTransform().GetPosition();
					currentPos += offset;
					parent->GetTransform().SetPosition(currentPos);

					// If, while falling (after applying the offset), the turret has a CollSide::Top collision
					// AND a negative z velocity (meaning it's falling down), then:
					if (side == CollSide::Top && mFallVelocity.z < 0.0f)
					{
						// Subtract 15 from the parent's z-position (this will make it appear more on the ground)
						currentPos = parent->GetTransform().GetPosition();
						currentPos.z -= 15.0f;
						parent->GetTransform().SetPosition(currentPos);

						// Call Die()
						Die();

						// Furthermore, if the CollSide::Top collision was against another TurretBase:
						TurretBase* otherTurret = dynamic_cast<TurretBase*>(collider);
						if (otherTurret)
						{
							// Adjust the parent's position further by subtracting 55 from the z-coordinate
							currentPos = parent->GetTransform().GetPosition();
							currentPos.z -= 55.0f;
							parent->GetTransform().SetPosition(currentPos);

							// Call Die on the other TurretBase
							otherTurret->Die();
						}
					}
				}
			}
		}
	}

	// 3. If the length of the fall velocity is greater than 800.0f, normalize it and multiply by 800.0f (for a terminal velocity)
	float velocityLength = mFallVelocity.Length();
	if (velocityLength > 800.0f)
	{
		mFallVelocity.Normalize();
		mFallVelocity *= 800.0f;
	}
}

void TurretHead::UpdateDead(float /*deltaTime*/)
{
	// Check for portal teleport first
	if (CheckPortalTeleport())
	{
		ChangeState(TurretState::Falling);
		return;
	}

	// Logic for Dead will be added later
}

// ------------------------
// State changing function
// ------------------------
void TurretHead::ChangeState(TurretState newState)
{
	// Check if new state is different from old one
	if (newState == mState)
	{
		return;
	}

	// If another VO sound is still playing, stop it first
	if (mCurrentVOSound.IsValid() &&
		gGame.GetAudio()->GetSoundState(mCurrentVOSound) == SoundState::Playing)
	{
		gGame.GetAudio()->StopSound(mCurrentVOSound);
	}

	mState = newState;
	mStateTimer = 0.0f;

	// Play sound from map corresponding to the new state
	auto soundIt = mStateSounds.find(newState);
	if (soundIt != mStateSounds.end())
	{
		mCurrentVOSound = gGame.GetAudio()->PlaySound(soundIt->second, false, this);
	}

	// When initially changing to the Firing state, reset the cooldown to 0.05
	if (newState == TurretState::Firing)
	{
		mFiringCooldown = 0.05f;
	}
}

// ------------------------
// Main Update
// ------------------------
void TurretHead::HandleUpdate(float deltaTime)
{
	// Update the timer first
	mStateTimer += deltaTime;

	// Update portal teleport cooldown
	if (mPortalTeleportCooldown > 0.0f)
	{
		mPortalTeleportCooldown -= deltaTime;
		if (mPortalTeleportCooldown < 0.0f)
		{
			mPortalTeleportCooldown = 0.0f;
		}
	}

	// State machine
	switch (mState)
	{
	case TurretState::Idle:
		UpdateIdle(deltaTime);
		break;

	case TurretState::Search:
		UpdateSearch(deltaTime);
		break;

	case TurretState::Priming:
		UpdatePriming(deltaTime);
		break;

	case TurretState::Firing:
		UpdateFiring(deltaTime);
		break;

	case TurretState::Falling:
		UpdateFalling(deltaTime);
		break;

	case TurretState::Dead:
		UpdateDead(deltaTime);
		break;
	}
}

// ------------------------
// Target acquisition
// ------------------------
bool TurretHead::CheckTargetAcquisition()
{
	// A target is acquired if:
	// 1. The laser's mLastHitActor is non-null
	// 2. AND the mLastHitActor has a HealthComponent
	// 3. AND the HealthComponent indicates it is not dead
	if (mLaserComp)
	{
		Actor* lastHit = mLaserComp->GetLastHitActor();
		if (lastHit)
		{
			// Check if the actor has a HealthComponent
			HealthComponent* health = lastHit->GetComponent<HealthComponent>();
			if (health && !health->IsDead())
			{
				// Save the acquired target
				mAcquiredTarget = lastHit;
				return true;
			}
		}
	}

	return false;
}

// ------------------------
// Portal teleporting
// ------------------------
bool TurretHead::CheckPortalTeleport()
{
	// If a turret teleports, it can't teleport again until 0.25 seconds elapse
	if (mPortalTeleportCooldown > 0.0f)
	{
		return false;
	}

	// You can't teleport unless there is both a blue and orange portal
	Portal* bluePortal = gGame.GetBluePortal();
	Portal* orangePortal = gGame.GetOrangePortal();
	if (!bluePortal || !orangePortal)
	{
		return false;
	}

	// To check for intersection with a portal, get the parent (TurretBase) and its CollisionComponent
	Actor* parent = GetTransform().GetParent();
	if (!parent)
	{
		return false;
	}

	CollisionComponent* parentColl = parent->GetComponent<CollisionComponent>();
	if (!parentColl)
	{
		return false;
	}

	Portal* entryPortal = nullptr;
	Portal* exitPortal = nullptr;

	// Check intersection with blue portal
	if (CollisionComponent* blueColl = bluePortal->GetComponent<CollisionComponent>();
		blueColl && parentColl->Intersect(blueColl))
	{
		entryPortal = bluePortal;
		exitPortal = orangePortal;
	}
	// Check intersection with orange portal
	else if (CollisionComponent* orangeColl = orangePortal->GetComponent<CollisionComponent>();
			 orangeColl && parentColl->Intersect(orangeColl))
	{
		entryPortal = orangePortal;
		exitPortal = bluePortal;
	}

	if (!entryPortal || !exitPortal)
	{
		return false;
	}

	// Set the parent's position to the position of the opposite portal
	Vector3 exitPos = exitPortal->GetTransform().GetPosition();
	parent->GetTransform().SetPosition(exitPos);

	// Add to the fall velocity a vector in the direction of the portal's forward with a magnitude of 250
	Vector3 exitForward = exitPortal->GetTransform().GetForward();
	mFallVelocity += exitForward * 250.0f;

	// Start cooldown
	mPortalTeleportCooldown = 0.25f;

	return true;
}

// ------------------------
// Death
// ------------------------
void TurretHead::Die()
{
	// Change the state to TurretState::Dead
	ChangeState(TurretState::Dead);

	// Set the parent's quaternion to one rotated about the x-axis by pi/2
	Actor* parent = GetTransform().GetParent();
	if (parent)
	{
		Quaternion rotX = Quaternion(Vector3::UnitX, Math::PiOver2);
		parent->GetTransform().SetQuat(rotX);
	}

	// Disable the laser component
	if (mLaserComp)
	{
		mLaserComp->SetEnabled(false);
	}
}

void TurretHead::TakeDamage()
{
	// The very first time this function gets called
	if (!mHasTakenDamage)
	{
		mHasTakenDamage = true;

		// Stop the current VO sound, if there is one playing
		if (mCurrentVOSound.IsValid() &&
			gGame.GetAudio()->GetSoundState(mCurrentVOSound) == SoundState::Playing)
		{
			gGame.GetAudio()->StopSound(mCurrentVOSound);
		}

		// Play TurretFriendlyFire.ogg on this actor
		gGame.GetAudio()->PlaySound("TurretFriendlyFire.ogg", false, this);
	}
}
