#include "EnergyCatcher.h"
#include "Game.h"
#include "MeshComponent.h"
#include "CollisionComponent.h"
#include "Renderer.h"
#include "Pellet.h"
#include "Door.h"
#include "EnergyLauncher.h"

EnergyCatcher::EnergyCatcher()
{
	mMesh = CreateComponent<MeshComponent>();
	mMesh->SetMesh(gGame.GetRenderer()->GetMesh("Assets/Meshes/EnergyCatcher.gpmesh"));

	mColl = CreateComponent<CollisionComponent>();
	mColl->SetSize(Vector3(50.0f, 50.0f, 50.0f));

	gGame.AddCollider(this);
}

EnergyCatcher::~EnergyCatcher()
{
	gGame.RemoveCollider(this);
}

void EnergyCatcher::CatchPellet(Pellet* pellet)
{
	if (mActivated || !pellet)
	{
		return;
	}

	mActivated = true;

	gGame.GetAudio()->PlaySound("EnergyCaught.ogg", false, this);

	// Position pellet 40 units in front of catcher (no quats needed)
	Transform& xf = GetTransform();
	Vector3 forward = xf.GetForward();
	Vector3 pos = xf.GetPosition() + forward * 40.0f;

	pellet->GetTransform().SetPosition(pos);
	pellet->SetVelocity(Vector3::Zero);

	// Open associated door, if any
	if (!mDoorName.empty())
	{
		if (Door* door = gGame.GetDoor(mDoorName))
		{
			door->Open();
		}

		// Any launcher tied to this door should stop firing
		for (Actor* actor : gGame.GetActors())
		{
			if (auto* launcher = dynamic_cast<EnergyLauncher*>(actor))
			{
				if (launcher->GetDoorName() == mDoorName)
				{
					launcher->SetDoorOpen(true);
				}
			}
		}
	}
}
