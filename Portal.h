#pragma once
#include "Actor.h"

class Portal : public Actor
{
public:
	Portal(class Game* game, int textureIndex);

	float GetOutYaw() const { return mOutYaw; }

protected:
	void OnUpdate(float deltaTime) override;

private:
	class PortalMeshComponent* mPortalMeshComponent;
	class CollisionComponent* mCollisionComponent;

	Matrix4 CalcViewMatrixBasic(Portal* oppositePortal);
	Matrix4 CalcViewMatrix(Portal* oppositePortal);
	Matrix4 OutPortalDirection(Portal* oppositePortal);

	Matrix4 mViewMatrix;

	float mOutYaw = 0.0f;
};