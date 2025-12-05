#pragma once
#include "Actor.h"
#include <string>

class MeshComponent;
class CollisionComponent;

class EnergyLauncher : public Actor
{
protected:
	EnergyLauncher();
	~EnergyLauncher() override;
	friend class Game;

	void HandleUpdate(float deltaTime) override;

public:
	void SetDoorName(const std::string& name) { mDoorName = name; }
	const std::string& GetDoorName() const { return mDoorName; }
	void SetDoorOpen(bool open) { mDoorOpen = open; }
	void SetCooldown(float cooldown) { mCooldown = cooldown; }

private:
	MeshComponent* mMesh = nullptr;
	CollisionComponent* mColl = nullptr;

	float mCooldown = 1.5f;
	float mTimeSinceShot = 0.0f;

	float mLauncherColliderSize = 50.0f;
	float mPelletSpeed = 500.0f;
	float mPelletSpawnOffset = 20.0f;

	std::string mDoorName;
	bool mDoorOpen = false;
};
