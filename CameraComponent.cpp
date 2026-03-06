#include "CameraComponent.h"
#include "Actor.h"
#include "Game.h"
#include "Renderer.h"
#include "Player.h"

CameraComponent::CameraComponent(class Actor* owner)
: Component(owner, 50)
{
}

void CameraComponent::Update(float deltaTime)
{
	mPitchAngle += mPitchSpeed * deltaTime;
	mPitchAngle = Math::Clamp(mPitchAngle, -Math::Pi / 2.1f, Math::Pi / 2.1f);
	Matrix4 rotationMatrix = Matrix4::CreateRotationY(mPitchAngle) *
							 Matrix4::CreateRotationZ(mOwner->GetRotation());
	Vector3 cameraForwardVector = Vector3::Transform(Vector3::UnitX, rotationMatrix);

	// Eye – position of camera in world space
	Vector3 eye = mOwner->GetPosition();

	// Target – point the camera “looks at” (also in world space)
	// TargetPos = OwnerPos + (OwnerForward * TargetDist)
	Vector3 targetPos = eye + cameraForwardVector * mTargetOffset;

	// Up – Up vector from the camera’s perspective
	Vector3 up = Vector3::UnitZ;

	Matrix4 viewMatrix = Matrix4::CreateLookAt(eye, targetPos, up);

	GetGame()->GetRenderer()->SetViewMatrix(viewMatrix);
}