//
// Created by Ryan Sahar on 11/20/25.
//
#include "Pellet.h"
#include "Game.h"
#include "MeshComponent.h"
#include "CollisionComponent.h"
#include "Player.h"
#include "Portal.h"
#include "EnergyCatcher.h"
#include "EnergyCube.h"
#include "EnergyGlass.h"
#include "HealthComponent.h"
#include "Renderer.h"

Pellet::Pellet()
{
	// Mesh, sphere w/ texture idx 1
	mMesh = CreateComponent<MeshComponent>();
	mMesh->SetMesh(gGame.GetRenderer()->GetMesh("Assets/Meshes/Sphere.gpmesh"));
	mMesh->SetTextureIndex(1);

	// Collision comp w/ size {25, 25, 25}
	mColl = CreateComponent<CollisionComponent>();
	mColl->SetSize(Vector3(25.0f, 25.0f, 25.0f));

	// Not a collider, so don't add to Game::mColliders
}

Pellet::~Pellet() = default;

void Pellet::HandleUpdate(float deltaTime)
{
	mAge += deltaTime;

	// Countdown teleport ignore window
	if (mTeleportIgnoreTime > 0.0f)
	{
		mTeleportIgnoreTime -= deltaTime;
		if (mTeleportIgnoreTime < 0.0f)
		{
			mTeleportIgnoreTime = 0.0f;
		}
	}

	// Move pellet along its velocity
	Transform& xform = GetTransform();
	Vector3 pos = xform.GetPosition();
	pos += mVelocity * deltaTime;
	xform.SetPosition(pos);

	CollisionComponent* self = GetComponent<CollisionComponent>();
	if (!self)
	{
		return;
	}

	// Always check collision w/ player
	Player* player = gGame.GetPlayer();
	if (player)
	{
		CollisionComponent* playerColl = player->GetComponent<CollisionComponent>();
		if (playerColl && self->Intersect(playerColl))
		{
			// Call TakeDamage on the player's HealthComponent
			HealthComponent* playerHealth = player->GetComponent<HealthComponent>();
			if (playerHealth)
			{
				Vector3 pelletPos = GetTransform().GetPosition();
				playerHealth->TakeDamage(100.0f, pelletPos);
			}
			gGame.GetAudio()->PlaySound("PelletDeath.ogg", false, this, false);
			Destroy();
			return;
		}
	}

	// If we're in post-teleport, skip portals and world colliders
	if (mTeleportIgnoreTime > 0.0f)
	{
		return;
	}

	// Pellets teleporting through portals
	if (TeleportThroughPortals())
	{
		// On the frame we teleport, ignore all other collisions
		return;
	}

	// For the first spawn immunity period, ignore world colliders
	if (mAge < SPAWN_IMMUNITY_TIME)
	{
		return;
	}

	// After 0.25s, colliding with any collider destroys the pellet,
	// EXCEPT when hitting an EnergyCatcher that then "catches" it.
	for (Actor* actor : gGame.GetColliders())
	{
		CollisionComponent* other = actor->GetComponent<CollisionComponent>();
		if (other && self->Intersect(other))
		{
			// Special case: EnergyCatcher
			if (auto* catcher = dynamic_cast<EnergyCatcher*>(actor))
			{
				// Only catch once, if not already activated
				if (!catcher->IsActivated())
				{
					catcher->CatchPellet(this);
				}

				// Pellet does NOT die when caught
				return;
			}

			// Special case: EnergyCube
			if (dynamic_cast<EnergyCube*>(actor) != nullptr)
			{
				// Don't die, turn green
				mIsGreen = true;
				if (mMesh)
				{
					mMesh->SetTextureIndex(2);
				}
				// Continue going through
				return;
			}

			// Special case: EnergyGlass
			if (dynamic_cast<EnergyGlass*>(actor) != nullptr)
			{
				// If green, go through (don't die)
				if (mIsGreen)
				{
					return;
				}
				// Otherwise, die
				gGame.GetAudio()->PlaySound("PelletDeath.ogg", false, this, false);
				Destroy();
				return;
			}

			// Check if the collider has a HealthComponent
			HealthComponent* health = actor->GetComponent<HealthComponent>();
			if (health)
			{
				// If the object is already dead, ignore the collision
				if (health->IsDead())
				{
					return;
				}

				// Otherwise, deal 100 damage
				Vector3 pelletPos = GetTransform().GetPosition();
				health->TakeDamage(100.0f, pelletPos);
			}

			// Any other collider kills the pellet
			gGame.GetAudio()->PlaySound("PelletDeath.ogg", false, this, false);
			Destroy();
			return;
		}
	}
}

bool Pellet::TeleportThroughPortals()
{
	// Need both portals for teleporting
	Portal* bluePortal = gGame.GetBluePortal();
	Portal* orangePortal = gGame.GetOrangePortal();
	if (!bluePortal || !orangePortal || !mColl)
	{
		return false;
	}

	Portal* entryPortal = nullptr;
	Portal* exitPortal = nullptr;

	// Check intersection w/ blue portal
	if (CollisionComponent* blueColl = bluePortal->GetComponent<CollisionComponent>();
		blueColl && mColl->Intersect(blueColl))
	{
		entryPortal = bluePortal;
		exitPortal = orangePortal;
	}

	// Check intersection w/ orange portal
	else if (CollisionComponent* orangeColl = orangePortal->GetComponent<CollisionComponent>();
			 orangeColl && mColl->Intersect(orangeColl))
	{
		entryPortal = orangePortal;
		exitPortal = bluePortal;
	}

	if (!entryPortal || !exitPortal)
	{
		return false;
	}

	// Transform position through the portals
	Transform& xform = GetTransform();
	Vector3 pos = xform.GetPosition();
	Vector3 newPos = entryPortal->GetPortalOutVector(pos, exitPortal, 1.0f);
	xform.SetPosition(newPos);

	// Transform velocity through the portals
	mVelocity = entryPortal->GetPortalOutVector(mVelocity, exitPortal, 0.0f);

	// After teleport, ignore portals and colliders
	mTeleportIgnoreTime = TELEPORT_IGNORE_TIME;

	return true;
}
