#include "PlayerMove.h"
#include "Actor.h"
#include "SDL2/SDL.h"
#include "Player.h"
#include "CameraComponent.h"
#include "Game.h"
#include "CollisionComponent.h"
#include "Crosshair.h"
#include "Renderer.h"
#include "SegmentCast.h"
#include "Block.h"
#include "Portal.h"
#include "HealthComponent.h"

PlayerMove::PlayerMove(Actor* owner)
: MoveComponent(owner)
{
	ChangeState(MoveState::OnGround);

	mCrosshair = new Crosshair(mOwner);

	mFootstepSound = GetGame()->GetAudio()->PlaySound("FootstepLoop.ogg", true);
	GetGame()->GetAudio()->PauseSound(mFootstepSound);
}

PlayerMove::~PlayerMove()
{
	GetGame()->GetAudio()->StopSound(mFootstepSound);
}

void PlayerMove::Update(float deltaTime)
{
	// If the player’s health component says the player is dead
	if (mOwner->GetComponent<HealthComponent>()->IsDead())
	{
		// Check if the mDeathSound is stopped
		if (GetGame()->GetAudio()->GetSoundState(static_cast<Player*>(mOwner)->GetDeathSound()) ==
			SoundState::Stopped)
		{
			// If it is, you should reload the level
			GetGame()->ReloadLevel();
		}
		// Otherwise, just return (so PlayerMove::Update won’t move the player anymore)
		else
		{
			return;
		}
	}

	switch (mCurrentState)
	{
	case MoveState::OnGround:
		UpdateOnGround(deltaTime);
		break;
	case MoveState::Jump:
		UpdateJump(deltaTime);
		break;
	case MoveState::Falling:
		UpdateFalling(deltaTime);
		break;
	}

	if (mOwner->GetPosition().z <= -750.0f)
	{
		static_cast<Player*>(mOwner)->GetComponent<HealthComponent>()->TakeDamage(
			Math::Infinity, static_cast<Player*>(mOwner)->GetPosition());
	}

	if (mTeleportTimer > 0.0f)
	{
		mTeleportTimer -= deltaTime;
	}

	if (mCurrentState == MoveState::OnGround && mVelocity.Length() > 50.0f)
	{
		GetGame()->GetAudio()->ResumeSound(mFootstepSound);
	}
	else
	{
		GetGame()->GetAudio()->PauseSound(mFootstepSound);
	}
}

void PlayerMove::ProcessInput(const Uint8* keyState, Uint32 mouseButtons,
							  const Vector2& relativeMouse)
{
	// Should not do anything if the player is dead.
	if (mOwner->GetComponent<HealthComponent>()->IsDead())
	{
		return;
	}

	// Update forward force

	if (keyState[SDL_SCANCODE_W])
	{
		AddForce(mOwner->GetForward() * mMovementSpeed);
	}
	if (keyState[SDL_SCANCODE_S])
	{
		AddForce(mOwner->GetForward() * -mMovementSpeed);
	}

	// Update strafe force

	if (keyState[SDL_SCANCODE_D])
	{
		AddForce(mOwner->GetRight() * mMovementSpeed);
	}
	if (keyState[SDL_SCANCODE_A])
	{
		AddForce(mOwner->GetRight() * -mMovementSpeed);
	}

	// Update angular speed

	float angularSpeed = relativeMouse.x / mMouseSpeed * Math::Pi * 10.0f;
	SetAngularSpeed(angularSpeed);

	// Update pitch speed

	float pitchSpeed = relativeMouse.y / mMouseSpeed * Math::Pi * 10.0f;
	static_cast<Player*>(mOwner)->GetCameraComponent()->SetPitchSpeed(pitchSpeed);

	// Jump

	if (keyState[SDL_SCANCODE_SPACE] && !mSpacePressed && mCurrentState == MoveState::OnGround)
	{
		AddForce(mJumpForce);
		ChangeState(MoveState::Jump);

		GetGame()->GetAudio()->PlaySound("Jump.ogg");
	}

	// Save the last frame value
	mSpacePressed = keyState[SDL_SCANCODE_SPACE];

	// Create Portal

	if (static_cast<Player*>(mOwner)->HasGun())
	{
		// Leading edge left click
		if (mouseButtons & SDL_BUTTON(SDL_BUTTON_LEFT) && !mLeftMousePressed)
		{
			// Create Blue Portal
			CreatePortal(true);
		}

		// Leading edge right click
		if (mouseButtons & SDL_BUTTON(SDL_BUTTON_RIGHT) && !mRightMousePressed)
		{
			// Create Orange Portal
			CreatePortal(false);
		}
	}

	// Store pressed for use in leading edge
	mLeftMousePressed = mouseButtons & SDL_BUTTON(SDL_BUTTON_LEFT);
	mRightMousePressed = mouseButtons & SDL_BUTTON(SDL_BUTTON_RIGHT);

	// Resetting Portals
	if (keyState[SDL_SCANCODE_R] && !mRPressed)
	{
		Portal* bluePortal = GetGame()->GetBluePortal();
		Portal* orangePortal = GetGame()->GetOrangePortal();

		if (bluePortal != nullptr || orangePortal != nullptr)
		{
			GetGame()->GetAudio()->PlaySound("PortalClose.ogg");
		}

		if (bluePortal != nullptr)
		{
			bluePortal->SetState(ActorState::Destroy);
			GetGame()->SetBluePortal(nullptr);
		}
		if (orangePortal != nullptr)
		{
			orangePortal->SetState(ActorState::Destroy);
			GetGame()->SetOrangePortal(nullptr);
		}
		mCrosshair->SetState(CrosshairState::Default);
	}

	mRPressed = keyState[SDL_SCANCODE_R];
}

void PlayerMove::UpdateOnGround(float deltaTime)
{
	PhysicsUpdate(deltaTime);
	if (UpdatePortalTeleport(deltaTime))
	{
		return;
	}

	bool standing = false;

	// Loop over the colliders
	// Prevent the player from walking through blocks
	for (Actor* collider : mOwner->GetGame()->GetColliders())
	{
		CollSide collSide = FixCollision(mOwner->GetComponent<CollisionComponent>(),
										 collider->GetComponent<CollisionComponent>());

		if (collSide == CollSide::Top)
		{
			standing = true;
		}
	}

	if (!standing)
	{
		ChangeState(MoveState::Falling);
	}
}

void PlayerMove::UpdateJump(float deltaTime)
{
	AddForce(mGravity);

	PhysicsUpdate(deltaTime);
	if (UpdatePortalTeleport(deltaTime))
	{
		return;
	}

	bool hitHead = false;

	// Loop over the colliders
	// Check if player hit their head on ceiling
	for (Actor* collider : mOwner->GetGame()->GetColliders())
	{
		CollSide collSide = FixCollision(mOwner->GetComponent<CollisionComponent>(),
										 collider->GetComponent<CollisionComponent>());

		if (collSide == CollSide::Bottom)
		{
			hitHead = true;
		}
	}

	if (hitHead)
	{
		mVelocity.z = 0.0f;
	}

	if (mVelocity.z <= 0.0f)
	{
		ChangeState(MoveState::Falling);
	}
}

void PlayerMove::UpdateFalling(float deltaTime)
{
	AddForce(mGravity);

	PhysicsUpdate(deltaTime);
	if (UpdatePortalTeleport(deltaTime))
	{
		return;
	}

	bool landed = false;

	// Loop over the colliders
	// Prevent the player from falling through the ground
	for (Actor* collider : mOwner->GetGame()->GetColliders())
	{
		CollSide collSide = FixCollision(mOwner->GetComponent<CollisionComponent>(),
										 collider->GetComponent<CollisionComponent>());

		if (collSide == CollSide::Top && mVelocity.z <= 0.0f)
		{
			landed = true;
		}
	}

	if (landed)
	{
		mVelocity.z = 0.0f;
		ChangeState(MoveState::OnGround);

		// For the bool you made to track if you're "falling due to portal teleport",
		// you should set this bool back to false when UpdateFalling changes states to OnGround
		mTeleportFalling = false;

		GetGame()->GetAudio()->PlaySound("Land.ogg");
	}
}

void PlayerMove::PhysicsUpdate(float deltaTime)
{
	// Newton says acceleration = Force / mass
	mAcceleration = mPendingForces * (1.0f / mMass);

	mVelocity += mAcceleration * deltaTime;
	FixXYVelocity();

	if (mVelocity.z < -1000.0f)
	{
		mVelocity.z = -1000.0f;
	}

	mOwner->SetPosition(mOwner->GetPosition() + mVelocity * deltaTime);

	mOwner->SetRotation(mOwner->GetRotation() + mAngularSpeed * deltaTime);

	// Set mPendingForces to Vector3::Zero (this means that every frame, we have to call AddForce for each
	// force we want to affect the next PhysicsUpdate)
	mPendingForces = Vector3::Zero;
}

void PlayerMove::FixXYVelocity()
{
	Vector2 xyVelocity = Vector2(mVelocity.x, mVelocity.y);
	float maxSpeed = 400.0f;

	// If the bool you made to track if you're "falling due to portal teleport" is true,
	// don't run the velocity limiting code in FixXYVelocity()
	if (!mTeleportFalling)
	{
		if (xyVelocity.Length() > maxSpeed)
		{
			// Change the length of xyVelocity to be exactly maxSpeed
			xyVelocity.Normalize();
			xyVelocity *= maxSpeed;
		}
	}

	// Break when letting go
	// Also allow for switching rapidly between two opposite directions
	if (mCurrentState == MoveState::OnGround)
	{
		if (Math::NearlyZero(mAcceleration.x) ||
			((mAcceleration.x < 0 && xyVelocity.x > 0) ||
			 (mAcceleration.x > 0 && xyVelocity.x < 0)) ||
			(Math::Abs(mAcceleration.x) < 0.0001))
		{
			xyVelocity.x *= 0.9f;
		}
		if (Math::NearlyZero(mAcceleration.y) ||
			((mAcceleration.y < 0 && xyVelocity.y > 0) ||
			 (mAcceleration.y > 0 && xyVelocity.y < 0)) ||
			(Math::Abs(mAcceleration.y) < 0.0001))
		{
			xyVelocity.y *= 0.9f;
		}
	}

	mVelocity.x = xyVelocity.x;
	mVelocity.y = xyVelocity.y;
}

CollSide PlayerMove::FixCollision(CollisionComponent* self, CollisionComponent* collider)
{
	Vector3 offset;
	CollSide collSide = self->GetMinOverlap(collider, offset);

	if (collSide != CollSide::None)
	{
		// fix the position of the player based on offset
		mOwner->SetPosition(mOwner->GetPosition() + offset);
	}

	return collSide;
}

void PlayerMove::CreatePortal(bool isBlue)
{
	Vector3 nearPlanePoint = GetGame()->GetRenderer()->Unproject(Vector3(0.0f, 0.0f, 0.0f));
	Vector3 farPlanePoint = GetGame()->GetRenderer()->Unproject(Vector3(0.0f, 0.0f, 1.0f));

	Vector3 direction = farPlanePoint - nearPlanePoint;
	direction.Normalize();

	Vector3 mStart = nearPlanePoint;
	// Set mEnd to the point that's 1000.0f units away from mStart in the normalized direction
	Vector3 mEnd = mStart + direction * 1000.0f;
	LineSegment lineSegment = LineSegment(mStart, mEnd);

	CastInfo outInfo;
	if (SegmentCast(GetGame()->GetColliders(), lineSegment, outInfo))
	{
		// Check if hit actor is block
		if (dynamic_cast<Block*>(outInfo.mActor))
		{
			Portal* portal = nullptr;

			if (isBlue)
			{
				Portal* bluePortal = GetGame()->GetBluePortal();
				portal = new Portal(GetGame(), 0);

				if (bluePortal != nullptr)
				{
					bluePortal->SetState(ActorState::Destroy);
				}

				GetGame()->SetBluePortal(portal);

				GetGame()->GetAudio()->PlaySound("PortalShootBlue.ogg");
			}
			else
			{
				Portal* orangePortal = GetGame()->GetOrangePortal();
				portal = new Portal(GetGame(), 1);

				if (orangePortal != nullptr)
				{
					orangePortal->SetState(ActorState::Destroy);
				}

				GetGame()->SetOrangePortal(portal);

				GetGame()->GetAudio()->PlaySound("PortalShootOrange.ogg");
			}

			portal->SetPosition(outInfo.mPoint);

			// Calculate the Rotation based on direction of surface

			Quaternion portalQuaternion;

			Vector3 originalFacing = Vector3::UnitX;
			Vector3 desiredFacing = outInfo.mNormal;

			float dotProduct = Vector3::Dot(originalFacing, desiredFacing);

			// If originalFacing and desiredFacing are collinear and facing in the same direction
			if (dotProduct == 1.0f)
			{
				// No rotation
				portalQuaternion = Quaternion::Identity;
			}
			// If originalFacing and desiredFacing are collinear and facing in the opposite direction
			else if (dotProduct == -1.0f)
			{
				// yaw of 180 degrees
				portalQuaternion = Quaternion(Vector3::UnitZ, Math::Pi);
			}
			else
			{
				Vector3 axisOfRotation = Vector3::Cross(originalFacing, desiredFacing);
				axisOfRotation.Normalize();

				// Find the angle of rotation (the angle between originalFacing and desiredFacing)
				float angleOfRotation = Math::Acos(dotProduct);

				portalQuaternion = Quaternion(axisOfRotation, angleOfRotation);
			}

			portal->SetQuat(portalQuaternion);

			// Set the portal collision component's size based on its surface

			CollisionComponent* cc = portal->GetComponent<CollisionComponent>();

			if (Math::Abs(outInfo.mNormal.x) == 1)
			{
				cc->SetSize(110.0f, 125.0f, 10.0f);
			}
			else if (Math::Abs(outInfo.mNormal.y) == 1)
			{
				cc->SetSize(10.0f, 125.0f, 110.0f);
			}
			else
			{
				cc->SetSize(110.0f, 10.0f, 125.0f);
			}

			// Set crosshair

			Portal* bluePortal = GetGame()->GetBluePortal();
			Portal* orangePortal = GetGame()->GetOrangePortal();

			if (bluePortal != nullptr && orangePortal != nullptr)
			{
				mCrosshair->SetState(CrosshairState::BothFill);
			}
			else if (bluePortal != nullptr)
			{
				mCrosshair->SetState(CrosshairState::BlueFill);
			}
			else if (orangePortal != nullptr)
			{
				mCrosshair->SetState(CrosshairState::OrangeFill);
			}
		}
		else
		{
			GetGame()->GetAudio()->PlaySound("PortalFail.ogg");
		}
	}
	else
	{
		GetGame()->GetAudio()->PlaySound("PortalFail.ogg");
	}
}

void PlayerMove::Reset()
{
	mVelocity = Vector3::Zero;
	mAcceleration = Vector3::Zero;
	mPendingForces = Vector3::Zero;
}

// returns true if the player teleports through a portal, and false otherwise
bool PlayerMove::UpdatePortalTeleport(float deltaTime)
{
	// You can only teleport if there's both a blue and orange portal
	if (GetGame()->GetBluePortal() != nullptr && GetGame()->GetOrangePortal() != nullptr)
	{
		// you can't teleport if it's been less than 0.2 seconds since a previous teleport
		if (mTeleportTimer <= 0.0f)
		{
			Portal* entryPortal = nullptr;
			Portal* exitPortal = nullptr;

			// You only initiate the teleport when you intersect with one of the portals
			if (mOwner->GetComponent<CollisionComponent>()->Intersect(
					GetGame()->GetBluePortal()->GetComponent<CollisionComponent>()))
			{
				entryPortal = GetGame()->GetBluePortal();
				exitPortal = GetGame()->GetOrangePortal();
			}
			else if (mOwner->GetComponent<CollisionComponent>()->Intersect(
						 GetGame()->GetOrangePortal()->GetComponent<CollisionComponent>()))
			{
				entryPortal = GetGame()->GetOrangePortal();
				exitPortal = GetGame()->GetBluePortal();
			}

			if (entryPortal != nullptr && exitPortal != nullptr)
			{
				// The player should change to the position of the exit portal
				mOwner->SetPosition(exitPortal->GetPosition());

				// Change the state to Falling
				mCurrentState = MoveState::Falling;

				// Change the player's velocity

				// Calculate the magnitude of the current velocity in the direction of the entry portal
				float entryMagnitude = Vector3::Dot(mVelocity, entryPortal->GetQuatForward()) *
									   -1.0f;

				// The magnitude of the exit velocity should be either 1.5m or 350.0f, whichever is higher
				float exitMagnitude = 350.0f;
				if (entryMagnitude * 1.5f > 350.0f)
				{
					exitMagnitude = entryMagnitude * 1.5f;
				}

				// The direction of the exit velocity should be in the direction of the exit portal
				mVelocity = exitMagnitude * exitPortal->GetQuatForward();

				// Set a bool that says you're falling due to a portal teleport
				mTeleportFalling = true;

				// Set the player's rotation angle so they face in the exit portal's direction
				// If the portal faces along the +/- x or +/- y axis
				if (Math::Abs(exitPortal->GetQuatForward().x) == 1 ||
					Math::Abs(exitPortal->GetQuatForward().y) == 1)
				{
					mOwner->SetRotation(entryPortal->GetOutYaw());
				}

				mTeleportTimer = mTeleportCooldown;

				GetGame()->GetAudio()->PlaySound("PortalTeleport.ogg");

				// returns true since you teleported
				return true;
			}
		}
	}

	return false;
}