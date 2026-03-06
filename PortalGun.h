#pragma once
#include "Actor.h"

class PortalGun : public Actor
{
public:
	PortalGun(class Game* game);

protected:
	void OnUpdate(float deltaTime) override;

private:
	class MeshComponent* mMeshComponent;
	class CollisionComponent* mCollisionComponent;
};