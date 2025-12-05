#pragma once
#include "Actor.h"
#include <string>

class MeshComponent;
class CollisionComponent;
class Pellet;

class EnergyCatcher : public Actor
{
protected:
	EnergyCatcher();
	~EnergyCatcher() override;
	friend class Game;
	friend class Pellet;

	void HandleUpdate(float /*deltaTime*/) override {}

public:
	bool IsActivated() const { return mActivated; }
	void SetDoorName(const std::string& name) { mDoorName = name; }

private:
	void CatchPellet(Pellet* pellet);

	MeshComponent* mMesh = nullptr;
	CollisionComponent* mColl = nullptr;

	bool mActivated = false;
	std::string mDoorName;
};
