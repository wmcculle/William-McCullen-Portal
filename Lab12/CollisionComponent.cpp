#include "CollisionComponent.h"
#include "Actor.h"
#include <algorithm>

CollisionComponent::CollisionComponent(class Actor* owner)
: Component(owner)
, mWidth(0.0f)
, mHeight(0.0f)
, mDepth(0.0f)
{
}

CollisionComponent::~CollisionComponent()
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
	Vector3 v = mOwner->GetPosition();
	v.x -= mDepth * mOwner->GetScale().x / 2.0f;
	v.y -= mWidth * mOwner->GetScale().y / 2.0f;
	v.z -= mHeight * mOwner->GetScale().z / 2.0f;
	return v;
}

Vector3 CollisionComponent::GetMax() const
{
	Vector3 v = mOwner->GetPosition();
	v.x += mDepth * mOwner->GetScale().x / 2.0f;
	v.y += mWidth * mOwner->GetScale().y / 2.0f;
	v.z += mHeight * mOwner->GetScale().z / 2.0f;
	return v;
}

Vector3 CollisionComponent::GetCenter() const
{
	return mOwner->GetPosition();
}

CollSide CollisionComponent::GetMinOverlap(const CollisionComponent* other, Vector3& offset) const
{
	offset = Vector3::Zero;

	// If they don't intersect, return
	if (!Intersect(other))
	{
		return CollSide::None;
	}

	// Get player min/max and block min/max
	Vector3 myMin = GetMin();
	Vector3 myMax = GetMax();
	Vector3 otherMin = other->GetMin();
	Vector3 otherMax = other->GetMax();

	// Figure out which side we are closest to
	// These differences are always "other" side minus "this" side
	float backDist = otherMin.x - myMax.x;
	float frontDist = otherMax.x - myMin.x;
	float leftDist = otherMin.y - myMax.y;
	float rightDist = otherMax.y - myMin.y;
	float bottomDist = otherMin.z - myMax.z;
	float topDist = otherMax.z - myMin.z;

	// Whichever of the six sides has the lowest absolute value is the side that is minimally overlapped
	float minDist = std::min({backDist, frontDist, leftDist, rightDist, bottomDist, topDist},
							 [](float a, float b) {
								 return Math::Abs(a) < Math::Abs(b);
							 });

	// Set the offset Vector3 passed in by reference to the offset to move this so it will be exactly touching other.
	// Return the CollSide that is the minimum overlap side
	if (minDist == backDist)
	{
		offset.x = minDist;
		return CollSide::Back;
	}
	else if (minDist == frontDist)
	{
		offset.x = minDist;
		return CollSide::Front;
	}
	else if (minDist == leftDist)
	{
		offset.y = minDist;
		return CollSide::Left;
	}
	else if (minDist == rightDist)
	{
		offset.y = minDist;
		return CollSide::Right;
	}
	else if (minDist == bottomDist)
	{
		offset.z = minDist;
		return CollSide::Bottom;
	}
	else //if (minDist == topDist)
	{
		offset.z = minDist;
		return CollSide::Top;
	}
}