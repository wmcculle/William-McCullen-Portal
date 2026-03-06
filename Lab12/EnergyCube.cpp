#include "EnergyCube.h"
#include "Game.h"
#include "MeshComponent.h"
#include "Renderer.h"
#include "Mesh.h"
#include "CollisionComponent.h"

EnergyCube::EnergyCube(Game* game)
: Actor(game)
{
	mMeshComponent = new MeshComponent(this);
	mMeshComponent->SetMesh(game->GetRenderer()->GetMesh("Assets/Meshes/EnergyCube.gpmesh"));

	mCollisionComponent = new CollisionComponent(this);
	mCollisionComponent->SetSize(25.0f, 25.0f, 25.0f);

	game->AddCollider(this);
}

EnergyCube::~EnergyCube()
{
	GetGame()->RemoveCollider(this);
}