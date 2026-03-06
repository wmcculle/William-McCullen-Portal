#pragma once
#include "Actor.h"

class EnergyCube : public Actor
{
public:
	EnergyCube(class Game* game);
	~EnergyCube();

private:
	class MeshComponent* mMeshComponent;
	class CollisionComponent* mCollisionComponent;
};