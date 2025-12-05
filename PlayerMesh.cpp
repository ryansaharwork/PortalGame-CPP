//
// Created by Ryan Sahar on 11/5/25.
//
#include "PlayerMesh.h"
#include "Game.h"
#include "MeshComponent.h"
#include "Renderer.h"
#include "Player.h"
#include "CameraComponent.h"
#include "Math.h"

PlayerMesh::PlayerMesh()
{
	// Set mesh
	mMesh = CreateComponent<MeshComponent>();
	mMesh->SetMesh(gGame.GetRenderer()->GetMesh("Assets/Meshes/PortalGun.gpmesh"));

	// Set scale
	GetTransform().SetScale({1.0f, 2.5f, 2.5f});
}

void PlayerMesh::HandleUpdate(float deltaTime)
{
	constexpr Vector3 SCREEN_PT(300.0f, -250.0f, 0.4f);
	Vector3 worldPos = gGame.GetRenderer()->Unproject(SCREEN_PT);
	GetTransform().SetPosition(worldPos);

	Player* player = gGame.GetPlayer();
	if (!player)
		return;

	float pitch = 0.0f;
	if (auto* cam = player->GetComponent<CameraComponent>())
	{
		pitch = cam->GetPitchAngle();
	}

	Quaternion qPitch(Vector3::UnitY, pitch);
	float yaw = player->GetTransform().GetRotation();
	Quaternion qYaw(Vector3::UnitZ, yaw);

	Quaternion q = Quaternion::Concatenate(qPitch, qYaw);
	GetTransform().SetQuat(q);
}
