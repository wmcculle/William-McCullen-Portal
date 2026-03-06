#include "EnergyLauncher.h"
#include "Game.h"
#include "MeshComponent.h"
#include "Renderer.h"
#include "Mesh.h"
#include "CollisionComponent.h"
#include "Pellet.h"
#include "MoveComponent.h"

EnergyLauncher::EnergyLauncher(Game* game, float pelletCooldown, std::string doorName)
: Actor(game)
, mPelletCooldown(pelletCooldown)
, mDoorName(doorName)
{
	mMeshComponent = new MeshComponent(this);
	mMeshComponent->SetMesh(game->GetRenderer()->GetMesh("Assets/Meshes/EnergyLauncher.gpmesh"));

	mCollisionComponent = new CollisionComponent(this);
	mCollisionComponent->SetSize(50.0f, 50.0f, 50.0f);

	game->AddCollider(this);
}

EnergyLauncher::~EnergyLauncher()
{
	GetGame()->RemoveCollider(this);
}

void EnergyLauncher::OnUpdate(float deltaTime)
{
	// The EnergyLauncher should fire pellets periodically based on its cooldown.
	if (mPelletCooldownTimer > 0.0f)
	{
		mPelletCooldownTimer -= deltaTime;
	}
	else if (mCanShoot)
	{
		mPelletCooldownTimer = mPelletCooldown;

		// To fire a pellet, the EnergyLauncher should create a new pellet actor 20 units in front of itself.
		Pellet* pellet = new Pellet(mGame);
		pellet->SetPosition(GetPosition() + GetForward() * 20.0f);

		// After setting the position of the new pellet actor, make sure you call CalcWorldTransform() on it.
		// Otherwise, the pellet will render at the origin on the first frame of its existence.
		pellet->CalcWorldTransform();

		// The pellet should start out moving in the forward direction of EnergyLauncher.
		// For simplicity, you may assume that EnergyLaunchers will only ever use GetForward() for their forward direction.
		pellet->SetQuat(Quaternion(Vector3::UnitZ, GetRotation()));

		mGame->GetAudio()->PlaySound("PelletFire.ogg", false, this);
	}
}