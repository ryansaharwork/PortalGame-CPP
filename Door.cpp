#include "Door.h"
#include "Game.h"
#include "MeshComponent.h"
#include "CollisionComponent.h"
#include "Renderer.h"
#include "Mesh.h"

Door::Door()
{
	// Mesh (door frame)
	mMesh = CreateComponent<MeshComponent>();
	Mesh* meshData = gGame.GetRenderer()->GetMesh("Assets/Meshes/DoorFrame.gpmesh");
	mMesh->SetMesh(meshData);

	// Collider sized from mesh box
	mColl = CreateComponent<CollisionComponent>();
	if (meshData)
	{
		mColl->SetSize(meshData->GetBoxSize());
	}

	// Start as closed collider
	gGame.AddCollider(this);

	// --- Child door halves ---

	// Left half
	mLeftDoor = CreateChild<Actor>();
	if (mLeftDoor)
	{
		MeshComponent* leftMesh = mLeftDoor->CreateComponent<MeshComponent>();
		leftMesh->SetMesh(gGame.GetRenderer()->GetMesh("Assets/Meshes/DoorLeft.gpmesh"));
	}

	// Right half
	mRightDoor = CreateChild<Actor>();
	if (mRightDoor)
	{
		MeshComponent* rightMesh = mRightDoor->CreateComponent<MeshComponent>();
		rightMesh->SetMesh(gGame.GetRenderer()->GetMesh("Assets/Meshes/DoorRight.gpmesh"));
	}
}

Door::~Door()
{
	// If still closed, remove from collider list
	if (!mIsOpen)
	{
		gGame.RemoveCollider(this);
	}
}

void Door::Open()
{
	if (!mIsOpen)
	{
		mIsOpen = true;
		mOpenTime = 0.0f;
		gGame.RemoveCollider(this); // "Open" door by removing collider
		gGame.GetAudio()->PlaySound("DoorOpen.ogg", false, this);
	}
}

void Door::HandleUpdate(float deltaTime)
{
	if (!mIsOpen)
	{
		return;
	}

	if (mOpenTime < 1.0f)
	{
		mOpenTime += deltaTime;
		if (mOpenTime > 1.0f)
		{
			mOpenTime = 1.0f;
		}

		const Vector3 START = Vector3::Zero;
		const Vector3 LEFT_END(0.0f, -100.0f, 0.0f);
		const Vector3 RIGHT_END(0.0f, 100.0f, 0.0f);

		Vector3 leftPos = Vector3::Lerp(START, LEFT_END, mOpenTime);
		Vector3 rightPos = Vector3::Lerp(START, RIGHT_END, mOpenTime);

		if (mLeftDoor)
		{
			mLeftDoor->GetTransform().SetPosition(leftPos);
		}
		if (mRightDoor)
		{
			mRightDoor->GetTransform().SetPosition(rightPos);
		}
	}
}
