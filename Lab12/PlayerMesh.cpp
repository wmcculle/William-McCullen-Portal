#include "PlayerMesh.h"
#include "Game.h"
#include "MeshComponent.h"
#include "Renderer.h"
#include "Mesh.h"
#include "Player.h"
#include "CameraComponent.h"

PlayerMesh::PlayerMesh(Game* game)
: Actor(game)
{
	mMeshComponent = new MeshComponent(this);
	mMeshComponent->SetMesh(game->GetRenderer()->GetMesh("Assets/Meshes/PortalGun.gpmesh"));

	SetScale(Vector3(1.0f, 2.5f, 2.5f));
}

void PlayerMesh::OnUpdate(float deltaTime)
{
	SetPosition(GetGame()->GetRenderer()->Unproject(Vector3(300.0f, -250.0f, 0.4f)));

	Quaternion pitchQuaternion =
		Quaternion(Vector3::UnitY, GetGame()->GetPlayer()->GetCameraComponent()->GetPitchAngle());

	Quaternion yawQuaternion = Quaternion(Vector3::UnitZ, GetGame()->GetPlayer()->GetRotation());

	SetQuat(Quaternion::Concatenate(pitchQuaternion, yawQuaternion));
}
