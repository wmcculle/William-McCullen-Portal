#pragma once
#include "MoveComponent.h"
#include "CollisionComponent.h"
#include "AudioSystem.h"

enum class MoveState
{
	OnGround,
	Jump,
	Falling
};

class PlayerMove : public MoveComponent
{
public:
	PlayerMove(class Actor* owner);
	~PlayerMove();

	const Vector3& GetVelocity() const { return mVelocity; }
	const Vector3& GetAcceleration() const { return mAcceleration; }

	void Reset();

protected:
	void Update(float deltaTime) override;
	void ProcessInput(const Uint8* keyState, Uint32 mouseButtons,
					  const Vector2& relativeMouse) override;

private:
	MoveState mCurrentState = MoveState::OnGround;

	void ChangeState(MoveState currentState) { mCurrentState = currentState; }

	void UpdateOnGround(float deltaTime);
	void UpdateJump(float deltaTime);
	void UpdateFalling(float deltaTime);

	Vector3 mVelocity;
	Vector3 mAcceleration;
	Vector3 mPendingForces; // the sum of all the forces we want to apply on a frame
	float mMass = 1.0f;

	void PhysicsUpdate(float deltaTime);
	void AddForce(const Vector3& force) { mPendingForces += force; }

	void FixXYVelocity();

	Vector3 mGravity = Vector3(0.0f, 0.0f, -980.0f);

	CollSide FixCollision(CollisionComponent* self, CollisionComponent* collider);

	Vector3 mJumpForce = Vector3(0.0f, 0.0f, 35000.0f);
	// Last frame value, used in calculating leading edge
	bool mSpacePressed = false;

	class Crosshair* mCrosshair;

	bool mLeftMousePressed = false;
	bool mRightMousePressed = false;

	void CreatePortal(bool isBlue);

	bool mRPressed = false;

	float mMovementSpeed = 700.0f;
	float mMouseSpeed = 500.0f;

	bool UpdatePortalTeleport(float deltaTime);

	float mTeleportTimer = 0.0f;
	float mTeleportCooldown = 0.2f;

	bool mTeleportFalling = false;

	SoundHandle mFootstepSound;
};