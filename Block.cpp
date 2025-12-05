//
// Created by Ryan Sahar on 11/4/25.
//
#include "Block.h"

#include "Mesh.h"
#include "MeshComponent.h"
#include "Game.h"
#include "Renderer.h"
#include "CollisionComponent.h"

Block::Block()
{
	GetTransform().SetScale(64.0f);

	mMesh = CreateComponent<MeshComponent>();
	mMesh->SetMesh(gGame.GetRenderer()->GetMesh("Assets/Meshes/Cube.gpmesh"));

	mColl = CreateComponent<CollisionComponent>();
	mColl->SetSize({1.0f, 1.0f, 1.0f});

	gGame.AddCollider(this);
}

Block::~Block()
{
	gGame.RemoveCollider(this);
}
