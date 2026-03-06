#include "EnergyGlass.h"
#include "Game.h"
#include "MeshComponent.h"
#include "Renderer.h"
#include "Mesh.h"
#include "CollisionComponent.h"

EnergyGlass::EnergyGlass(Game* game)
: Actor(game)
{
	mMeshComponent = new MeshComponent(this, true);
	mMeshComponent->SetMesh(game->GetRenderer()->GetMesh("Assets/Meshes/Cube.gpmesh"));
	mMeshComponent->SetTextureIndex(17);

	mCollisionComponent = new CollisionComponent(this);
	mCollisionComponent->SetSize(1.0f, 1.0f, 1.0f);

	game->AddCollider(this);
}

EnergyGlass::~EnergyGlass()
{
	GetGame()->RemoveCollider(this);
}