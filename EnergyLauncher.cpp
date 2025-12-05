#include "EnergyLauncher.h"
#include "Game.h"
#include "MeshComponent.h"
#include "CollisionComponent.h"
#include "Pellet.h"
#include "Renderer.h"
#include "Transform.h"

EnergyLauncher::EnergyLauncher()
{
	mMesh = CreateComponent<MeshComponent>();
	mMesh->SetMesh(gGame.GetRenderer()->GetMesh("Assets/Meshes/EnergyLauncher.gpmesh"));

	mColl = CreateComponent<CollisionComponent>();
	mColl->SetSize(Vector3(mLauncherColliderSize, mLauncherColliderSize, mLauncherColliderSize));

	gGame.AddCollider(this);
}

EnergyLauncher::~EnergyLauncher()
{
	gGame.RemoveCollider(this);
}

void EnergyLauncher::HandleUpdate(float deltaTime)
{
	// If the associated door is open, stop firing
	if (mDoorOpen)
	{
		return;
	}

	mTimeSinceShot += deltaTime;
	if (mTimeSinceShot < mCooldown)
	{
		return;
	}
	mTimeSinceShot = 0.0f;

	Vector3 forward = GetTransform().GetForward();
	Vector3 spawnPos = GetTransform().GetPosition() + forward * mPelletSpawnOffset;

	Pellet* pellet = gGame.CreateActor<Pellet>();
	pellet->GetTransform().SetPosition(spawnPos);
	pellet->GetTransform().SetQuat(GetTransform().GetQuat());
	pellet->SetVelocity(forward * mPelletSpeed);

	gGame.GetAudio()->PlaySound("PelletFire.ogg", false, this);
}
