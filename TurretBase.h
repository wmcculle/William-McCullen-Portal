#pragma once
#include "Actor.h"

class TurretBase : public Actor
{
public:
	TurretBase(class Game* game);
	~TurretBase();

	void Die();

private:
	class MeshComponent* mMeshComponent;
	class CollisionComponent* mCollisionComponent;
	class TurretHead* mHead;
	class HealthComponent* mHealthComponent;
};