#include "Prop.h"
#include "Game.h"
#include "MeshComponent.h"
#include "Renderer.h"
#include "Mesh.h"
#include "CollisionComponent.h"

Prop::Prop(Game* game, std::string meshName, bool usesAlpha, bool usesCollision)
: Actor(game)
{
	mMeshComponent = new MeshComponent(this, usesAlpha);
	Mesh* mesh = game->GetRenderer()->GetMesh(meshName);
	mMeshComponent->SetMesh(mesh);

	if (usesCollision)
	{
		mCollisionComponent = new CollisionComponent(this);
		mCollisionComponent->SetSize(mesh->GetWidth(), mesh->GetHeight(), mesh->GetDepth());
		game->AddCollider(this);
	}
}

Prop::~Prop()
{
	if (mCollisionComponent != nullptr)
	{
		GetGame()->RemoveCollider(this);
	}
}