#include "MoveComponent.h"
#include "Actor.h"

MoveComponent::MoveComponent(class Actor* owner)
: Component(owner, 50)
, mAngularSpeed(0.0f)
, mForwardSpeed(0.0f)
{
}

void MoveComponent::Update(float deltaTime)
{
	// Update rotation
	mOwner->SetRotation(mOwner->GetRotation() + mAngularSpeed * deltaTime);

	// Update forward movement
	Vector3 forwardVelocity = mOwner->GetForward() * mForwardSpeed;
	mOwner->SetPosition(mOwner->GetPosition() + forwardVelocity * deltaTime);

	// Update strafe movement
	Vector3 strafeVelocity = mOwner->GetRight() * mStrafeSpeed;
	mOwner->SetPosition(mOwner->GetPosition() + strafeVelocity * deltaTime);
}
