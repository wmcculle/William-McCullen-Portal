#pragma once
#include "MeshComponent.h"
#include "Math.h"
#include "SegmentCast.h"
#include <vector>

class Actor;

class LaserComponent : public MeshComponent
{
public:
	LaserComponent(Actor* owner);

	void Update(float deltaTime) override;

	// Draw this mesh component
	void Draw(class Shader* shader) override;

	void SetIgnoreActor(Actor* ignoreActor) { mIgnoreActor = ignoreActor; }

	Actor* GetLastHitActor() const { return mLastHitActor; }

	void SetDisabled();

private:
	std::vector<LineSegment> mLineSegments;

	Matrix4 CalcLaserTransform(LineSegment lineSegment);

	Actor* mIgnoreActor = nullptr;

	Vector3 OutPortalDirection(Vector3 initialDirection, class Portal* oppositePortal);

	Actor* mLastHitActor = nullptr;

	bool mIsDisabled = false;
};
