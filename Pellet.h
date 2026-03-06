#pragma once
#include "Actor.h"

class Pellet : public Actor
{
public:
	Pellet(class Game* game);

protected:
	void OnUpdate(float deltaTime) override;

private:
	class MeshComponent* mMeshComponent;
	class CollisionComponent* mCollisionComponent;

	float mPelletSpeed = 500.0f;

	// For its first 0.25 seconds of existence, Pellet should not collide with colliders
	float mInvincibilityTimer = 0.25f;

	void Destroy();
};