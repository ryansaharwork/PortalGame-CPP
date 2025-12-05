//
// Created by Ryan Sahar on 12/1/25.
//
#pragma once

#include "MeshComponent.h"
#include "SegmentCast.h"
#include <vector>

class Actor;

class LaserComponent : public MeshComponent
{
protected:
	LaserComponent(class Actor* owner);
	friend class Actor;

	void HandleUpdate(float deltaTime) override;
	void Draw(class Shader* shader) override;

public:
	// SegmentCast should ignore this actor when casting
	void SetIgnoreActor(class Actor* actor) { mIgnoreActor = actor; }

	// Actor hit by the last laser this frame (may be nullptr)
	Actor* GetLastHitActor() const { return mLastHitActor; }

	// Enable/disable the laser
	void SetEnabled(bool enabled) { mIsEnabled = enabled; }
	bool IsEnabled() const { return mIsEnabled; }

private:
	std::vector<LineSegment> mSegments;
	class Actor* mIgnoreActor = nullptr;

	// Actor that was hit by the last laser this frame
	class Actor* mLastHitActor = nullptr;

	// Whether the laser is enabled
	bool mIsEnabled = true;

	// Helper: build a world transform for a given line segment
	Matrix4 GetSegmentTransform(const LineSegment& segment) const;
};
