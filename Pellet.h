#pragma once
#include "Actor.h"

class MeshComponent;
class CollisionComponent;

class Pellet : public Actor
{
public:
	// Called by energy launcher to set initial velocity
	void SetVelocity(const Vector3& vel) { mVelocity = vel; }

protected:
	Pellet();
	~Pellet() override;
	friend class Game;

	void HandleUpdate(float deltaTime) override;

private:
	// Teleport through portals if intersecting one
	bool TeleportThroughPortals();

	// Tunable gameplay constants
	static constexpr float SPAWN_IMMUNITY_TIME = 0.25f;
	static constexpr float TELEPORT_IGNORE_TIME = 0.25f;

	MeshComponent* mMesh = nullptr;
	CollisionComponent* mColl = nullptr;

	Vector3 mVelocity = Vector3::Zero;
	float mAge = 0.0f;				  // Tracks lifetime for spawn protection
	float mTeleportIgnoreTime = 0.0f; // Ignore portals/colliders after teleport
	bool mIsGreen = false;			  // Track if pellet is green (can go through energy glass)
};
