#pragma once
#include "Actor.h"

class EnergyGlass : public Actor
{
public:
	EnergyGlass(class Game* game);
	~EnergyGlass();

private:
	class MeshComponent* mMeshComponent;
	class CollisionComponent* mCollisionComponent;
};