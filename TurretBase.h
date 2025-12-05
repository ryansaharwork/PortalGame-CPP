#pragma once

#include "Actor.h"

class MeshComponent;
class CollisionComponent;
class TurretHead;
class HealthComponent;

class TurretBase : public Actor
{
protected:
	TurretBase();
	~TurretBase() override;
	friend class Game;

public:
	void Die() const;

private:
	MeshComponent* mMesh = nullptr;
	CollisionComponent* mColl = nullptr;
	HealthComponent* mHealth = nullptr;

	// Child turret head (spinning part that owns the laser)
	TurretHead* mHead = nullptr;
};
