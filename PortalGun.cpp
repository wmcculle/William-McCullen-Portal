#include "PortalGun.h"
#include "Game.h"
#include "MeshComponent.h"
#include "Renderer.h"
#include "Mesh.h"
#include "CollisionComponent.h"

PortalGun::PortalGun(Game* game)
: Actor(game)
{
	mMeshComponent = new MeshComponent(this);
	mMeshComponent->SetMesh(game->GetRenderer()->GetMesh("Assets/Meshes/PortalGun.gpmesh"));

	mCollisionComponent = new CollisionComponent(this);
	mCollisionComponent->SetSize(8.0f, 8.0f, 8.0f);
}

void PortalGun::OnUpdate(float deltaTime)
{
	float rotationRate = Math::Pi;
	SetRotation(GetRotation() + rotationRate * deltaTime);
}