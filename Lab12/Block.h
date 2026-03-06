#pragma once
#include "Actor.h"

class Block : public Actor
{
public:
	Block(class Game* game);
	~Block();

private:
	class MeshComponent* mMeshComponent;
	class CollisionComponent* mCollisionComponent;
};