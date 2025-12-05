#include "CollisionComponent.h"
#include "Actor.h"
#include "Math.h"

namespace
{
	// Scale factor for half-extents of the collision box
	constexpr float HALF_EXTENT_SCALE = 0.5f;
} // namespace

CollisionComponent::CollisionComponent(class Actor* owner)
: Component(owner)
{
}

bool CollisionComponent::Intersect(const CollisionComponent* other) const
{
	Vector3 thisMin = GetMin();
	Vector3 thisMax = GetMax();
	Vector3 otherMin = other->GetMin();
	Vector3 otherMax = other->GetMax();

	bool noIntersection = thisMax.x < otherMin.x || thisMax.y < otherMin.y ||
						  thisMax.z < otherMin.z || otherMax.x < thisMin.x ||
						  otherMax.y < thisMin.y || otherMax.z < thisMin.z;

	return !noIntersection;
}

Vector3 CollisionComponent::GetMin() const
{
	Vector3 v = GetCenter();
	Vector3 scale = mOwner->GetTransform().GetScale();

	// Subtract half-extents in each axis
	v.x -= mSize.x * scale.x * HALF_EXTENT_SCALE;
	v.y -= mSize.y * scale.y * HALF_EXTENT_SCALE;
	v.z -= mSize.z * scale.z * HALF_EXTENT_SCALE;

	return v;
}

Vector3 CollisionComponent::GetMax() const
{
	Vector3 v = GetCenter();
	Vector3 scale = mOwner->GetTransform().GetScale();

	// Add half-extents in each axis
	v.x += mSize.x * scale.x * HALF_EXTENT_SCALE;
	v.y += mSize.y * scale.y * HALF_EXTENT_SCALE;
	v.z += mSize.z * scale.z * HALF_EXTENT_SCALE;

	return v;
}

Vector3 CollisionComponent::GetCenter() const
{
	return mOwner->GetTransform().GetPosition();
}

CollSide CollisionComponent::GetMinOverlap(const CollisionComponent* other, Vector3& offset) const
{
	offset = Vector3::Zero;

	// Early out if not intersecting
	if (!Intersect(other))
	{
		return CollSide::None;
	}

	// AABB mins/maxes (already account for scale in GetMin/GetMax)
	const Vector3 A_MIN = GetMin();
	const Vector3 A_MAX = GetMax();
	const Vector3 B_MIN = other->GetMin();
	const Vector3 B_MAX = other->GetMax();

	// Distances per axis:
	// X axis -> Front/Back
	const float BACK_DIST = B_MIN.x - A_MAX.x;	// hit other's Back (−x)
	const float FRONT_DIST = B_MAX.x - A_MIN.x; // hit other's Front (+x)

	// Y-axis -> Left/Right
	const float LEFT_DIST = B_MIN.y - A_MAX.y;	// hit other's Left (−y)
	const float RIGHT_DIST = B_MAX.y - A_MIN.y; // hit other's Right (+y)

	// Z axis -> Bottom/Top
	const float BOTTOM_DIST = B_MIN.z - A_MAX.z; // hit other's Bottom (−z)
	const float TOP_DIST = B_MAX.z - A_MIN.z;	 // hit other's Top (+z)

	auto absf = [](float v) {
		return v < 0.0f ? -v : v;
	};

	// Pick the minimum |distance| (same tie behavior as 2D version)
	CollSide side = CollSide::Back; // baseline
	float minA = absf(BACK_DIST);

	float a = absf(FRONT_DIST);
	if (a < minA)
	{
		side = CollSide::Front;
		minA = a;
	}

	a = absf(LEFT_DIST);
	if (a < minA)
	{
		side = CollSide::Left;
		minA = a;
	}

	a = absf(RIGHT_DIST);
	if (a < minA)
	{
		side = CollSide::Right;
		minA = a;
	}

	a = absf(BOTTOM_DIST);
	if (a < minA)
	{
		side = CollSide::Bottom;
		minA = a;
	}

	a = absf(TOP_DIST);
	if (a < minA)
	{
		side = CollSide::Top;
		// minA = a; // not needed since we don't use minA after this
	}

	// Set offset along the chosen axis
	switch (side)
	{
	case CollSide::Back:
		offset.x = BACK_DIST; // −x
		break;
	case CollSide::Front:
		offset.x = FRONT_DIST; // +x
		break;
	case CollSide::Left:
		offset.y = LEFT_DIST; // −y
		break;
	case CollSide::Right:
		offset.y = RIGHT_DIST; // +y
		break;
	case CollSide::Bottom:
		offset.z = BOTTOM_DIST; // −z
		break;
	case CollSide::Top:
		offset.z = TOP_DIST; // +z
		break;
	default:
		break;
	}

	return side;
}
