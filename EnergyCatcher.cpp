#include "EnergyCatcher.h"
#include "Game.h"
#include "MeshComponent.h"
#include "Renderer.h"
#include "Mesh.h"
#include "CollisionComponent.h"
#include "Door.h"
#include "EnergyLauncher.h"

EnergyCatcher::EnergyCatcher(Game* game, std::string doorName)
: Actor(game)
, mDoorName(doorName)
{
	mMeshComponent = new MeshComponent(this);
	mMeshComponent->SetMesh(game->GetRenderer()->GetMesh("Assets/Meshes/EnergyCatcher.gpmesh"));

	mCollisionComponent = new CollisionComponent(this);
	mCollisionComponent->SetSize(50.0f, 50.0f, 50.0f);

	game->AddCollider(this);
}

EnergyCatcher::~EnergyCatcher()
{
	GetGame()->RemoveCollider(this);
}

void EnergyCatcher::SetActivated()
{
	mActivated = true;

	// when an EnergyCatcher becomes activated it opens the door associated with it (if any)
	if (!mDoorName.empty())
	{
		// To “open” a door, for now just remove the door from the collider vector (we’ll add a simple animation for this in the next lab).
		for (Actor* collider : mGame->GetColliders())
		{
			Door* door = dynamic_cast<Door*>(collider);
			if (door != nullptr && door->GetDoorName() == mDoorName)
			{
				mGame->RemoveCollider(door);
				door->SetIsOpen(true);
			}
		}

		// Then, also make it so EnergyLaunchers no longer shoot pellets if the door they’re associated with (if any) is open.
		for (Actor* collider : mGame->GetColliders())
		{
			EnergyLauncher* energyLauncher = dynamic_cast<EnergyLauncher*>(collider);
			if (energyLauncher != nullptr && energyLauncher->GetDoorName() == mDoorName)
			{
				energyLauncher->DisableShoot();
			}
		}
	}

	mGame->GetAudio()->PlaySound("EnergyCaught.ogg", false, this);
}