#pragma once
#include "Actor.h"

class PlayerMesh : public Actor
{
public:
	PlayerMesh(class Game* game);

protected:
	void OnUpdate(float deltaTime) override;

private:
	class MeshComponent* mMeshComponent;
};