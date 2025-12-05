//
// Created by Ryan Sahar on 11/5/25.
//
#include "PortalGun.h"
#include "Game.h"
#include "Mesh.h"
#include "MeshComponent.h"
#include "Renderer.h"
#include "CollisionComponent.h"
#include "Math.h"
#include "Player.h"

PortalGun::PortalGun()
{
	mMesh = CreateComponent<MeshComponent>();
	mMesh->SetMesh(gGame.GetRenderer()->GetMesh("Assets/Meshes/PortalGun.gpmesh"));

	mColl = CreateComponent<CollisionComponent>();
	mColl->SetSize({8.0f, 8.0f, 8.0f});
}
void PortalGun::HandleUpdate(float deltaTime)
{
	Actor::HandleUpdate(deltaTime);

	// Rotate using actor's rotation angle
	float rotation = GetTransform().GetRotation();
	rotation += Math::Pi * deltaTime;
	GetTransform().SetRotation(rotation);

	// Give player gun
	Player* player = gGame.GetPlayer();
	if (player && mColl)
	{
		if (CollisionComponent* pColl = player->GetComponent<CollisionComponent>())
		{
			if (mColl->Intersect(pColl))
			{
				player->GiveGun();
				gGame.AddPendingDestroy(this);
				return;
			}
		}
	}
}
