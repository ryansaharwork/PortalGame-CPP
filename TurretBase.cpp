#include "TurretBase.h"

#include "MeshComponent.h"
#include "CollisionComponent.h"
#include "Game.h"
#include "Renderer.h"
#include "TurretHead.h"
#include "LaserComponent.h"
#include "HealthComponent.h"

TurretBase::TurretBase()
{
	// Base scale
	GetTransform().SetScale(0.75f);

	// Mesh for the base
	mMesh = CreateComponent<MeshComponent>();
	mMesh->SetMesh(gGame.GetRenderer()->GetMesh("Assets/Meshes/TurretBase.gpmesh"));

	// Collider for the base
	mColl = CreateComponent<CollisionComponent>();
	mColl->SetSize(Vector3(25.0f, 25.0f, 110.0f));

	// Health component so turrets can be targeted
	mHealth = CreateComponent<HealthComponent>();

	// Set up death callback to call Die()
	mHealth->SetOnDeath([this] {
		Die();
	});

	// Set up damage callback to call TakeDamage on the turret head
	mHealth->SetOnDamage([this](const Vector3& /*location*/) {
		if (mHead)
		{
			mHead->TakeDamage();
		}
	});

	// Add this base to the collider vector
	gGame.AddCollider(this);

	// Create the turret head as a child
	mHead = CreateChild<TurretHead>();

	// Hook the laser to ignore this base (so it doesn't instantly collide with itself)
	if (mHead && mHead->GetLaserComponent())
	{
		mHead->GetLaserComponent()->SetIgnoreActor(this);
	}
}

TurretBase::~TurretBase()
{
	// Remove from collider vector when destroyed
	gGame.RemoveCollider(this);
}

void TurretBase::Die() const
{
	// Call Die on the turret head
	if (mHead)
	{
		mHead->Die();
	}
}
