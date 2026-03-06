#include "TurretBase.h"
#include "Game.h"
#include "MeshComponent.h"
#include "Renderer.h"
#include "Mesh.h"
#include "CollisionComponent.h"
#include "TurretHead.h"
#include "HealthComponent.h"

TurretBase::TurretBase(Game* game)
: Actor(game)
{
	SetScale(0.75f);

	mMeshComponent = new MeshComponent(this);
	mMeshComponent->SetMesh(game->GetRenderer()->GetMesh("Assets/Meshes/TurretBase.gpmesh"));

	mCollisionComponent = new CollisionComponent(this);
	mCollisionComponent->SetSize(25.0f, 110.0f, 25.0f);

	game->AddCollider(this);

	mHead = new TurretHead(game, this);

	mHealthComponent = new HealthComponent(this);

	// For the turret’s HealthComponent, set the mOnDeath callback so that it calls Die on the turret.
	mHealthComponent->SetOnDeath([this] {
		Die();
	});

	mHealthComponent->SetOnDamage([this](const Vector3& location) {
		mHead->TakeDamage();
	});
}

TurretBase::~TurretBase()
{
	GetGame()->RemoveCollider(this);
}

void TurretBase::Die()
{
	mHead->Die();
}