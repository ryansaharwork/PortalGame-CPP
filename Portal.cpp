//
// Created by Ryan Sahar on 10/30/25.
//
#include "Portal.h"

#include "AlphaMeshComponent.h"
#include "CameraComponent.h"
#include "CollisionComponent.h"
#include "Game.h"
#include "Player.h"
#include "PortalMeshComponent.h"
#include "Renderer.h"
#include "Math.h"

void Portal::Setup(const Vector3& pos, const Vector3& normal, bool isBlue)
{
	mIsBlue = isBlue;

	// Portal surface
	PortalMeshComponent* portalComp = CreateComponent<PortalMeshComponent>();
	portalComp->SetTextureIndex(isBlue ? 0 : 1);

	// Outline
	AlphaMeshComponent* outline = CreateComponent<AlphaMeshComponent>();
	outline->SetMesh(gGame.GetRenderer()->GetMesh("Assets/Meshes/Portal.gpmesh"));
	outline->SetTextureIndex(isBlue ? 2 : 3);

	// Set pos
	GetTransform().SetPosition(pos);

	CollisionComponent* coll = CreateComponent<CollisionComponent>();

	// Determine coll size based on axis the normal aligns with
	Vector3 size;
	if (Math::NearlyEqual(fabsf(normal.x), 1.0f))
	{
		// +/- X
		size = Vector3(10.0f, 110.0f, 125.0f);
	}
	else if (Math::NearlyEqual(fabsf(normal.y), 1.0f))
	{
		// +/- Y
		size = Vector3(110.0f, 10.0f, 125.0f);
	}
	else
	{
		// +/- Z
		size = Vector3(125.0f, 110.0f, 10.0f);
	}

	coll->SetSize(size);

	// PORTAL ROTATION LOGIC
	constexpr Vector3 ORIGINAL_FACING = Vector3::UnitX; // Default forward
	Vector3 desiredFacing = Vector3::Normalize(normal);

	// Find dot/edge cases
	float dot = Vector3::Dot(ORIGINAL_FACING, desiredFacing);

	Quaternion q;
	if (Math::NearlyEqual(dot, 1.0f))
	{
		// Facing right direction already
		q = Quaternion::Identity;
	}

	else if (Math::NearlyEqual(dot, -1.0f))
	{
		// Opposite direction
		q = Quaternion(Vector3::UnitZ, Math::Pi);
	}

	else
	{
		// Normal case
		Vector3 axis = Vector3::Normalize(Vector3::Cross(ORIGINAL_FACING, desiredFacing));
		float angle = Math::Acos(dot);
		q = Quaternion(axis, angle);
	}

	GetTransform().SetQuat(q);

	HandleUpdate(0.0f);
}

Vector3 Portal::GetPortalOutVector(const Vector3& inVec, const Portal* exitPortal, float w) const
{
	// STEP 1: Inverse world transform of the *entry* portal
	Matrix4 entryWorld = const_cast<Portal*>(this)->GetTransform().GetWorldTransform();
	entryWorld.Invert();

	// STEP 2: Transform initial vector into entry portal object space
	Vector3 entryLocal = Vector3::Transform(inVec, entryWorld, w);

	// STEP 3: Create a Z-rotation matrix rotating by Math::Pi
	Matrix4 rotZ = Matrix4::CreateRotationZ(Math::Pi);

	// STEP 4: Rotate the local vector by π about Z
	Vector3 rotatedLocal = Vector3::Transform(entryLocal, rotZ, w);

	// STEP 5: Transform by the *exit* portal's world transform
	Matrix4 exitWorld = const_cast<Portal*>(exitPortal)->GetTransform().GetWorldTransform();
	Vector3 outVec = Vector3::Transform(rotatedLocal, exitWorld, w);

	// STEP 6: Return the transformed vector
	return outVec;
}

void Portal::HandleUpdate(float deltaTime)
{
	if (mIsBlue)
	{
		// Blue portal: use blue PortalData and orange as the exit
		CalcViewMatrix(gGame.GetRenderer()->GetBluePortal(), gGame.GetOrangePortal());
	}
	else
	{
		// Orange portal: use orange PortalData and blue as the exit
		CalcViewMatrix(gGame.GetRenderer()->GetOrangePortal(), gGame.GetBluePortal());
	}
}
Portal::Portal()
{
}

void Portal::CalcViewMatrix(struct PortalData& portalData, Portal* exitPortal) const
{
	// STEP 1: No exit portal: use invalid view matrix so nothing is rendered
	if (!exitPortal)
	{
		portalData.mView = Matrix4::CreateScale(0.0f);
		return;
	}

	// STEP 2a: Portal view camera position = player's position transformed through portals
	Player* player = gGame.GetPlayer();
	Vector3 playerPos = player->GetTransform().GetPosition();
	Vector3 camPos = GetPortalOutVector(playerPos, exitPortal, 1.0f);

	// STEP 2b: Portal view camera forward = player's camera forward transformed through portals
	CameraComponent* cam = player->GetComponent<CameraComponent>();
	float pitch = cam->GetPitchAngle();
	float yaw = player->GetTransform().GetRotation();

	Matrix4 pitchMat = Matrix4::CreateRotationY(pitch);
	Matrix4 yawMat = Matrix4::CreateRotationZ(yaw);
	Matrix4 combo = pitchMat * yawMat;

	Vector3 playerForward = Vector3::Transform(Vector3::UnitX, combo);
	Vector3 camForward = GetPortalOutVector(playerForward, exitPortal, 0.0f);

	// STEP 2c: Portal view camera up = Z axis of exit portal's world transform
	const Matrix4& exitWorld = exitPortal->GetTransform().GetWorldTransform();
	Vector3 camUp = exitWorld.GetZAxis();

	// STEP 2d: Build look-at matrix (target is 50 units in front of camera)
	Vector3 target = camPos + camForward * 50.0f;
	Matrix4 view = Matrix4::CreateLookAt(camPos, target, camUp);

	// STEP 2e: Fill out portalData
	portalData.mView = view;
	portalData.mCameraPos = camPos;
	portalData.mCameraForward = camForward;
	portalData.mCameraUp = camUp;
}
