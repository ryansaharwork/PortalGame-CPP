//
// Created by Ryan Sahar on 11/20/25.
//
#include "EnergyGlass.h"
#include "Game.h"
#include "AlphaMeshComponent.h"
#include "CollisionComponent.h"
#include "Renderer.h"

EnergyGlass::EnergyGlass()
{
	AlphaMeshComponent* mesh = CreateComponent<AlphaMeshComponent>();
	mesh->SetMesh(gGame.GetRenderer()->GetMesh("Assets/Meshes/Cube.gpmesh"));
	mesh->SetTextureIndex(17);

	CollisionComponent* coll = CreateComponent<CollisionComponent>();
	coll->SetSize(Vector3(1.0f, 1.0f, 1.0f));

	// Is a collider
	gGame.AddCollider(this);
}

EnergyGlass::~EnergyGlass()
{
	// Remove from collider list
	gGame.RemoveCollider(this);
}
