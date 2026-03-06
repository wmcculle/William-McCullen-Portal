#include "Door.h"
#include "Game.h"
#include "MeshComponent.h"
#include "Renderer.h"
#include "Mesh.h"
#include "CollisionComponent.h"

Door::Door(Game* game, std::string doorName)
: Actor(game)
, mDoorName(doorName)
{
	mMeshComponent = new MeshComponent(this);
	Mesh* mesh = game->GetRenderer()->GetMesh("Assets/Meshes/DoorFrame.gpmesh");
	mMeshComponent->SetMesh(mesh);

	// Has collision component with a size from the mesh GetWidth(), GetHeight(), GetDepth()
	mCollisionComponent = new CollisionComponent(this);
	mCollisionComponent->SetSize(mesh->GetWidth(), mesh->GetHeight(), mesh->GetDepth());

	game->AddCollider(this);

	// doors

	mDoorLeftHalf = new Actor(game, this);
	MeshComponent* doorLeftHalfMeshComponent = new MeshComponent(mDoorLeftHalf);
	mesh = game->GetRenderer()->GetMesh("Assets/Meshes/DoorLeft.gpmesh");
	doorLeftHalfMeshComponent->SetMesh(mesh);

	mDoorRightHalf = new Actor(game, this);
	MeshComponent* doorRightHalfMeshComponent = new MeshComponent(mDoorRightHalf);
	mesh = game->GetRenderer()->GetMesh("Assets/Meshes/DoorRight.gpmesh");
	doorRightHalfMeshComponent->SetMesh(mesh);
}

Door::~Door()
{
	GetGame()->RemoveCollider(this);
}

void Door::OnUpdate(float deltaTime)
{
	if (mIsOpen)
	{
		if (mOpenTimer < mOpenTime)
		{
			mOpenTimer += deltaTime;
		}

		mDoorLeftHalf->SetPosition(
			Vector3::Lerp(Vector3::Zero, Vector3(0.0f, -100.0f, 0.0f), mOpenTimer));
		mDoorRightHalf->SetPosition(
			Vector3::Lerp(Vector3::Zero, Vector3(0.0f, 100.0f, 0.0f), mOpenTimer));
	}
}

void Door::SetIsOpen(bool isOpen)
{
	mIsOpen = isOpen;

	mGame->GetAudio()->PlaySound("DoorOpen.ogg", false, this);
}