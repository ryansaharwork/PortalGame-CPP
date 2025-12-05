//
// Created by Ryan Sahar on 11/4/25.
//
#include "Prop.h"

#include "AlphaMeshComponent.h"
#include "Game.h"
#include "Mesh.h"
#include "MeshComponent.h"
#include "Renderer.h"
#include "CollisionComponent.h"

Prop::Prop() = default;
Prop::~Prop()
{
	// If collision is enabled, get rid of this prop from collider list
	if (mColl)
	{
		gGame.RemoveCollider(this);
	}
}

void Prop::SetMesh(const std::string& meshFile, bool usesAlpha)
{
	// Create the right mesh component (based on Alpha)
	if (usesAlpha)
	{
		mMeshComp = CreateComponent<AlphaMeshComponent>();
	}

	else
	{
		mMeshComp = CreateComponent<MeshComponent>();
	}

	// Load and set the mesh
	mMesh = gGame.GetRenderer()->GetMesh(meshFile);
	if (mMeshComp)
	{
		mMeshComp->SetMesh(mMesh);
	}
}
void Prop::EnableCollision()
{
	mColl = CreateComponent<CollisionComponent>();
	Vector3 boxSize = mMesh->GetBoxSize();
	mColl->SetSize(boxSize);

	// Registered w/ game's collider list
	// since this has collision on
	gGame.AddCollider(this);
}
