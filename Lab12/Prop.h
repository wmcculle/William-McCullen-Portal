#pragma once
#include "Actor.h"

class Prop : public Actor
{
public:
	Prop(class Game* game, std::string meshName, bool usesAlpha, bool usesCollision);
	~Prop();

private:
	class MeshComponent* mMeshComponent;
	class CollisionComponent* mCollisionComponent = nullptr;
};