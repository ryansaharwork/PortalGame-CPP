#pragma once
#include "Actor.h"
#include <string>

class MeshComponent;
class CollisionComponent;

class Door : public Actor
{
protected:
	Door();
	~Door() override;
	friend class Game;

	void HandleUpdate(float deltaTime) override;

public:
	void Open();
	bool IsOpen() const { return mIsOpen; }

	void SetName(const std::string& name) { mName = name; }
	const std::string& GetName() const { return mName; }

private:
	MeshComponent* mMesh = nullptr;
	CollisionComponent* mColl = nullptr;

	// Child actors for the two door halves
	Actor* mLeftDoor = nullptr;
	Actor* mRightDoor = nullptr;

	bool mIsOpen = false;
	float mOpenTime = 0.0f; // 0 = closed, 1 = fully open

	std::string mName;
};
