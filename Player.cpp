//
// Created by Ryan Sahar on 10/21/25.
//
#include "Player.h"
#include "PlayerMove.h"
#include "CameraComponent.h"
#include "CollisionComponent.h"
#include "Game.h"
#include "PlayerMesh.h"
#include "HealthComponent.h"
#include "HUD.h"
#include "Random.h"
#include <vector>

bool Player::HasGun() const
{
	return mHasGun;
}

void Player::GiveGun()
{
	mHasGun = true;
	gGame.CreateActor<PlayerMesh>(); // Spawn fp mesh
}

Player::Player()
{
	mPlayerMove = CreateComponent<PlayerMove>();
	mCamera = CreateComponent<CameraComponent>();
	mColl = CreateComponent<CollisionComponent>();
	mColl->SetSize({50.0f, 50.0f, 100.0f});

	// Health component so player can be targeted by turrets
	mHealth = CreateComponent<HealthComponent>();

	mDeathSound = SoundHandle::Invalid;

	// Set up death callback to play taunt and show subtitle
	mHealth->SetOnDeath([this] {
		std::vector<std::string> sounds = {"Glados-PlayerDead1.ogg", "Glados-PlayerDead2.ogg",
										   "Glados-PlayerDead3.ogg", "Glados-PlayerDead4.ogg"};
		std::vector<std::string> subtitles = {
			"Congratulations! The test is now over.",
			"Thank you for participating in this Aperture Science computer-aided enrichment activity.",
			"Goodbye.", "You're not a good person. You know that, right?"};

		int index = Random::GetIntRange(0, 3);
		mDeathSound = gGame.GetAudio()->PlaySound(sounds[index], false);

		if (mHUD)
		{
			mHUD->ShowSubtitle(subtitles[index]);
		}
	});

	mHUD = CreateComponent<HUD>();

	// Set up damage callback to show damage indicator
	mHealth->SetOnDamage([this](const Vector3& location) {
		// Get the vector from the player to the location, set z to 0 and normalize
		Vector3 toLocation = location - GetTransform().GetPosition();
		toLocation.z = 0.0f;
		toLocation.Normalize();

		// Get the forward vector of the player, set z to 0 and normalize
		Vector3 forward = GetTransform().GetForward();
		forward.z = 0.0f;
		forward.Normalize();

		// Calculate the angle between them using cross product to determine sign
		float dot = Vector3::Dot(forward, toLocation);
		dot = Math::Clamp(dot, -1.0f, 1.0f);
		float angle = Math::Acos(dot);

		Vector3 cross = Vector3::Cross(forward, toLocation);
		if (cross.z > 0.0f)
		{
			angle = -angle;
		}

		// Call PlayerTakeDamage on the HUD
		if (mHUD)
		{
			mHUD->PlayerTakeDamage(angle);
		}
	});
}
