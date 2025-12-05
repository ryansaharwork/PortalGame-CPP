//
// Created by Ryan Sahar on 10/21/25.
//
#pragma once
#include "Actor.h"
#include "AudioSystem.h"

class CollisionComponent;
class PlayerMove;
class CameraComponent;
class HealthComponent;
class HUD;

class Player : public Actor
{
public:
	bool HasGun() const;
	void GiveGun();

	void SetInitialPos(const Vector3& pos) { mInitialPos = pos; }
	Vector3 GetInitialPos() const { return mInitialPos; }

	HUD* GetHUD() const { return mHUD; }
	SoundHandle GetDeathSound() const { return mDeathSound; }

protected:
	Player();
	friend class Game;

private:
	PlayerMove* mPlayerMove = nullptr;
	CameraComponent* mCamera = nullptr;
	CollisionComponent* mColl = nullptr;
	HealthComponent* mHealth = nullptr;
	HUD* mHUD = nullptr;

	bool mHasGun = false; // Defaults to no gun

	Vector3 mInitialPos = Vector3::Zero;
	SoundHandle mDeathSound;
};
