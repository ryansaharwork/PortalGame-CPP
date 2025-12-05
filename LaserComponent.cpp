//
// Created by Ryan Sahar on 12/1/25.
//

#include "LaserComponent.h"
#include "Actor.h"
#include "Game.h"
#include "Renderer.h"
#include "Mesh.h"
#include "Shader.h"
#include "Texture.h"
#include "VertexArray.h"
#include "Portal.h"

LaserComponent::LaserComponent(class Actor* owner)
: MeshComponent(owner)
{
	// Use the laser mesh
	SetMesh(gGame.GetRenderer()->GetMesh("Assets/Meshes/Laser.gpmesh"));
}

void LaserComponent::HandleUpdate(float /*deltaTime*/)
{
	// If disabled, clear the laser vector and prevent creation of line segments
	if (!mIsEnabled)
	{
		mSegments.clear();
		mLastHitActor = nullptr;
		return;
	}

	// 1. Clear out the line segment vector
	mSegments.clear();

	// Reset last hit each frame
	mLastHitActor = nullptr;
	Actor* lastHit = nullptr;

	// FIRST SEGMENT
	// From owner's position, 350 units along owner's forward
	Vector3 start = mOwner->GetTransform().GetPosition();
	Vector3 dir = mOwner->GetTransform().GetForward();
	Vector3 end = start + dir * 350.0f;

	LineSegment seg(start, end);

	CastInfo info;
	bool hitSomething = SegmentCast(gGame.GetActors(), seg, info, mIgnoreActor);

	Portal* entryPortal = nullptr;
	Portal* exitPortal = nullptr;

	if (hitSomething)
	{
		// Shorten to collision point
		seg.mEnd = info.mPoint;
		lastHit = info.mActor;

		// Check if we hit a portal and both portals exist
		entryPortal = dynamic_cast<Portal*>(info.mActor);
		Portal* blue = gGame.GetBluePortal();
		Portal* orange = gGame.GetOrangePortal();

		if (entryPortal && blue && orange)
		{
			if (entryPortal == blue)
			{
				exitPortal = orange;
			}
			else if (entryPortal == orange)
			{
				exitPortal = blue;
			}
		}
	}

	// Store the first (possibly shortened) segment
	mSegments.emplace_back(seg);

	// SECOND SEGMENT (through portal)
	if (entryPortal && exitPortal)
	{
		// 1) Transform direction vector of first segment
		Vector3 segDir = Vector3::Normalize(seg.mEnd - seg.mStart);
		Vector3 outDir = entryPortal->GetPortalOutVector(segDir, exitPortal, 0.0f);
		outDir.Normalize();

		// 2) Transform collision point and offset 5.5 units along outDir
		Vector3 teleportedPoint = entryPortal->GetPortalOutVector(info.mPoint, exitPortal, 1.0f);
		Vector3 secondStart = teleportedPoint + outDir * 5.5f;

		// 3) Remaining length is 350 minus length of first segment
		float firstLen = seg.Length();
		float remaining = 350.0f - firstLen;
		if (remaining < 0.0f)
		{
			remaining = 0.0f;
		}

		Vector3 secondEnd = secondStart + outDir * remaining;
		LineSegment secondSeg(secondStart, secondEnd);

		// 4) SegmentCast again, but ignore the *exit* portal this time
		CastInfo info2;
		if (SegmentCast(gGame.GetActors(), secondSeg, info2, exitPortal))
		{
			secondSeg.mEnd = info2.mPoint;
			lastHit = info2.mActor;
		}

		mSegments.emplace_back(secondSeg);
	}

	// Finalize last hit for this frame
	mLastHitActor = lastHit;

	// Safety: if the last hit actor is a portal, clear it out (portal may be deleted)
	if (dynamic_cast<Portal*>(mLastHitActor))
	{
		mLastHitActor = nullptr;
	}
}

Matrix4 LaserComponent::GetSegmentTransform(const LineSegment& segment) const
{
	// scale * rotation * translation

	// Scale: x = length, y/z = 1
	float length = segment.Length();
	Matrix4 matScale = Matrix4::CreateScale(length, 1.0f, 1.0f);

	// Rotation: face along segment direction
	constexpr Vector3 ORIGINAL_FACING = Vector3::UnitX; // laser mesh points +X
	Vector3 dir = Vector3::Normalize(segment.mEnd - segment.mStart);
	float dot = Vector3::Dot(ORIGINAL_FACING, dir);

	Quaternion q;
	if (Math::NearlyEqual(dot, 1.0f))
	{
		q = Quaternion::Identity;
	}
	else if (Math::NearlyEqual(dot, -1.0f))
	{
		q = Quaternion(Vector3::UnitZ, Math::Pi);
	}
	else
	{
		Vector3 axis = Vector3::Normalize(Vector3::Cross(ORIGINAL_FACING, dir));
		float angle = Math::Acos(dot);
		q = Quaternion(axis, angle);
	}

	Matrix4 matRot = Matrix4::CreateFromQuaternion(q);

	// Translation: center of the segment
	Vector3 center = segment.PointOnSegment(0.5f);
	Matrix4 matTrans = Matrix4::CreateTranslation(center);

	return matScale * matRot * matTrans;
}

void LaserComponent::Draw(Shader* shader)
{
	// Loop over every line segment and draw the laser mesh for each one
	for (const LineSegment& segment : mSegments)
	{
		if (mMesh)
		{
			// Use the segment-based world transform instead of the owner's
			Matrix4 world = GetSegmentTransform(segment);
			shader->SetMatrixUniform("uWorldTransform", world);

			// Set the active texture
			Texture* t = mMesh->GetTexture(mTextureIndex);
			if (t)
			{
				t->SetActive();
			}

			// Set the mesh's vertex array as active
			VertexArray* va = mMesh->GetVertexArray();
			va->SetActive();

			// Draw
			glDrawElements(GL_TRIANGLES, static_cast<int>(va->GetNumIndices()), GL_UNSIGNED_INT,
						   nullptr);
		}
	}
}
