//
// Created by Ryan Sahar on 11/20/25.
//
#include "EnergyCube.h"
#include "Game.h"
#include "MeshComponent.h"
#include "CollisionComponent.h"
#include "Renderer.h"

EnergyCube::EnergyCube()
{
	MeshComponent* mesh = CreateComponent<MeshComponent>();
	mesh->SetMesh(gGame.GetRenderer()->GetMesh("Assets/Meshes/EnergyCube.gpmesh"));

	CollisionComponent* coll = CreateComponent<CollisionComponent>();
	coll->SetSize(Vector3(25.0f, 25.0f, 25.0f));

	// Is a collider
	gGame.AddCollider(this);
}

EnergyCube::~EnergyCube()
{
	// Remove from a collider list
	gGame.RemoveCollider(this);
}
