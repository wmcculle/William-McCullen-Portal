#include "TurretHead.h"
#include "Game.h"
#include "MeshComponent.h"
#include "Renderer.h"
#include "Mesh.h"
#include "CollisionComponent.h"
#include "LaserComponent.h"
#include "HealthComponent.h"
#include "Random.h"
#include "Portal.h"
#include "TurretBase.h"

TurretHead::TurretHead(Game* game, Actor* parent)
: Actor(game, parent)
{
	SetScale(0.75f);

	SetPosition(Vector3(0.0f, 0.0f, 18.75f));

	mMeshComponent = new MeshComponent(this);
	mMeshComponent->SetMesh(game->GetRenderer()->GetMesh("Assets/Meshes/TurretHead.gpmesh"));

	mLaser = new Actor(game, this);
	mLaser->SetPosition(Vector3(0.0f, 0.0f, 10.0f));

	mLaserComponent = new LaserComponent(mLaser);
	mLaserComponent->SetIgnoreActor(parent);

	SearchRandomPoint();

	// Fill the sound map
	mSoundMap[TurretState::Idle] = "TurretIdle.ogg";
	mSoundMap[TurretState::Search] = "TurretSearch.ogg";
	mSoundMap[TurretState::Priming] = "TurretPriming.ogg";
	mSoundMap[TurretState::Firing] = "TurretFiring.ogg";
	mSoundMap[TurretState::Falling] = "TurretFalling.ogg";
	mSoundMap[TurretState::Dead] = "TurretDead.ogg";
}

void TurretHead::OnUpdate(float deltaTime)
{
	if (mTeleportCooldown > 0.0f)
	{
		mTeleportCooldown -= deltaTime;
	}

	mStateTimer += deltaTime;

	switch (mTurretState)
	{
	case TurretState::Idle:
		UpdateIdle(deltaTime);
		break;
	case TurretState::Search:
		UpdateSearch(deltaTime);
		break;
	case TurretState::Priming:
		UpdatePriming(deltaTime);
		break;
	case TurretState::Firing:
		UpdateFiring(deltaTime);
		break;
	case TurretState::Falling:
		UpdateFalling(deltaTime);
		break;
	case TurretState::Dead:
		UpdateDead(deltaTime);
		break;
	}
}

void TurretHead::UpdateIdle(float deltaTime)
{
	// Do a check for portal teleport
	if (PortalTeleport())
	{
		// If the teleport occurs, you want to change states to falling and don’t do any of the other logic in the function.
		ChangeState(TurretState::Falling);
		return;
	}

	// In the Idle state, if a target is acquired, switch to the Priming state.
	if (TargetAcquisition())
	{
		ChangeState(TurretState::Priming);
	}
}

void TurretHead::UpdateSearch(float deltaTime)
{
	// Do a check for portal teleport
	if (PortalTeleport())
	{
		// If the teleport occurs, you want to change states to falling and don’t do any of the other logic in the function.
		ChangeState(TurretState::Falling);
		return;
	}

	// If while in the Search state, a target is acquired, switch to Priming.
	if (TargetAcquisition())
	{
		ChangeState(TurretState::Priming);
		return;
	}
	// The will turret turn to face the random point on the oval and then turn back to facing the center over the course of one second.
	// Then find another random point and repeat while still in Search state.

	// Once you have the quaternion, use Quaternion::Slerp to interpolate from
	// Quaternion::Identity to the arbitrary quaternion you calculated over half a second
	if (mInterpolateForward)
	{
		if (mTurnTimer < 0.5f)
		{
			mTurnTimer += deltaTime;
		}
		else
		{
			mInterpolateForward = false;
		}
	}
	// After this finishes, interpolate back to identity over half a second
	else
	{
		if (mTurnTimer > 0.0f)
		{
			mTurnTimer -= deltaTime;
		}
		else
		{
			mInterpolateForward = true;
			SearchRandomPoint();

			// If more than 5 seconds have elapsed in the search state, instead of starting a new cycle,
			// have the turret return to the Idle state.
			if (mStateTimer >= 5.0f)
			{
				ChangeState(TurretState::Idle);
			}
		}
	}
	mTurnTimer = Math::Clamp(mTurnTimer, 0.0f, 0.5f);
	SetQuat(Quaternion::Slerp(Quaternion::Identity, mQuatTurretToPoint, mTurnTimer * 2.0f));
}

void TurretHead::SearchRandomPoint()
{
	// Pick a random point to turn towards
	float sideDist = 75.0f;
	float upDist = 25.0f;
	float fwdDist = 200.0f;
	Vector3 fwd = Vector3::UnitX;

	// Center = TurretHeadPos + FwdDist * Fwd
	Vector3 center = GetPosition() + fwdDist * fwd;

	float randomAngle = Random::GetFloatRange(-Math::Pi, Math::Pi);

	Vector3 offset = Vector3(0.0f, sideDist * Math::Cos(randomAngle),
							 upDist * Math::Sin(randomAngle));

	// Add this offset to center to get a point on the oval
	Vector3 point = center + offset;

	// Once you have a point on the oval, make a unit vector from the turret to that point
	Vector3 unitTurretToPoint = point - GetPosition();
	unitTurretToPoint.Normalize();

	// Turn that into a quaternion
	float dotProduct = Vector3::Dot(fwd, unitTurretToPoint);

	//------
	Vector3 axisOfRotation = Vector3::Cross(fwd, unitTurretToPoint);
	axisOfRotation.Normalize();

	// Find the angle of rotation (the angle between originalFacing and desiredFacing)
	float angleOfRotation = Math::Acos(dotProduct);

	mQuatTurretToPoint = Quaternion(axisOfRotation, angleOfRotation);
}

void TurretHead::UpdatePriming(float deltaTime)
{
	// Do a check for portal teleport
	if (PortalTeleport())
	{
		// If the teleport occurs, you want to change states to falling and don’t do any of the other logic in the function.
		ChangeState(TurretState::Falling);
		return;
	}

	// If the last hit actor is not still the acquired target, switch to the Search state.
	if (mAcquiredTarget != mLaserComponent->GetLastHitActor())
	{
		ChangeState(TurretState::Search);
	}
	// Else if at least 1.5 seconds has elapsed in Priming, switch to Firing.
	else if (mStateTimer >= 1.5f)
	{
		// Make sure that when you initialy change to the Firing state, you reset the cooldown to 0.05.
		mDamageCooldown = 0.05f;

		ChangeState(TurretState::Firing);
	}
}

void TurretHead::UpdateFiring(float deltaTime)
{
	// Do a check for portal teleport
	if (PortalTeleport())
	{
		// If the teleport occurs, you want to change states to falling and don’t do any of the other logic in the function.
		ChangeState(TurretState::Falling);
		return;
	}

	// If the last hit actor is no longer the acquired target, then switch to Search
	if (mAcquiredTarget != mLaserComponent->GetLastHitActor())
	{
		ChangeState(TurretState::Search);
		return;
	}

	// Logic to “fire” at the target:

	// If the target is still valid (meaning you didn’t switch to search)
	// And the health component doesn’t say the target is dead
	if (!mAcquiredTarget->GetComponent<HealthComponent>()->IsDead())
	{
		if (mDamageCooldown >= 0.0f)
		{
			mDamageCooldown -= deltaTime;
		}
		else
		{
			// Deal damage every 0.05 seconds.
			mDamageCooldown = 0.05f;

			// Every time the turret deals damage, it should deal 2.5 damage and use the GetWorldPostion() as the
			// location where the damage is coming from.
			mAcquiredTarget->GetComponent<HealthComponent>()->TakeDamage(2.5f, GetWorldPosition());

			mGame->GetAudio()->PlaySound("Bullet.ogg", false, mAcquiredTarget);
		}
	}
}

void TurretHead::UpdateFalling(float deltaTime)
{
	// Make the turret fall

	// Update the parent’s position according to the fall velocity and delta time
	GetParent()->SetPosition(GetParent()->GetPosition() + mFallVelocity * deltaTime);

	// If you don’t teleport
	if (!PortalTeleport())
	{
		// Update velocity according to a gravity of <0, 0, -980> and delta time.
		mFallVelocity += Vector3(0.0f, 0.0f, -980.0f) * deltaTime;

		for (Actor* collider : mGame->GetColliders())
		{
			// You don’t want to collide against the parent
			if (collider != GetParent())
			{
				// Use GetMinOverlap between the parent’s collision and all colliders
				Vector3 offset;
				CollSide collSide = GetParent()->GetComponent<CollisionComponent>()->GetMinOverlap(
					collider->GetComponent<CollisionComponent>(), offset);

				// Update positions based on offset like usual
				if (collSide != CollSide::None)
				{
					// fix the position of the turret based on offset
					GetParent()->SetPosition(GetParent()->GetPosition() + offset);

					// If while falling, after applying the offset, check if the turret has a CollSide::Top of AND a negative z
					// velocity (meaning it’s falling down):
					if (collSide == CollSide::Top && mFallVelocity.z <= 0)
					{
						// Subtract 15 from the parent’s z-position (which will make it appear more on the ground).
						GetParent()->SetPosition(GetParent()->GetPosition() -
												 Vector3(0.0f, 0.0f, 15.0f));

						// Call Die()
						Die();

						// If the CollSide::Top collision was against another TurretBase
						TurretBase* turretBase = dynamic_cast<TurretBase*>(collider);
						if (turretBase != nullptr)
						{
							// Adjust the parent’s position further by subtracting the collision component height / 2 from the z
							GetParent()->SetPosition(
								GetParent()->GetPosition() -
								Vector3(
									0.0f, 0.0f,
									GetParent()->GetComponent<CollisionComponent>()->GetHeight() /
										2.0f));

							// Call Die on the other TurretBase
							turretBase->Die();
						}
					}
				}
			}
		}
	}

	// If the length of the fall velocity is greater than 800.0f
	if (mFallVelocity.Length() > 800.0f)
	{
		// Normalize it and multiply by 800.0f (for a terminal velocity)
		mFallVelocity.Normalize();
		mFallVelocity *= 800.0f;
	}
}

void TurretHead::UpdateDead(float deltaTime)
{
	// Do a check for portal teleport
	if (PortalTeleport())
	{
		// If the teleport occurs, you want to change states to falling and don’t do any of the other logic in the function.
		ChangeState(TurretState::Falling);
		return;
	}
}

void TurretHead::ChangeState(TurretState turretState)
{
	if (turretState != mTurretState)
	{
		// If another VO sound is still playing, stop it first
		if (mGame->GetAudio()->GetSoundState(mCurrentVO) == SoundState::Playing)
		{
			mGame->GetAudio()->StopSound(mCurrentVO);
		}

		// Play the sound from the map corresponding to the state you’re changing to, passing in this for
		// the third parameter to PlaySound, and save the handle in the member variable
		mCurrentVO = mGame->GetAudio()->PlaySound(mSoundMap[turretState], false, this);
	}

	mTurretState = turretState;
	mStateTimer = 0.0f;
}

bool TurretHead::TargetAcquisition()
{
	Actor* lastHitActor = mLaserComponent->GetLastHitActor();

	// Do not acquire a target if it’s health component says it’s already dead
	if (lastHitActor != nullptr && lastHitActor->GetComponent<HealthComponent>() &&
		!lastHitActor->GetComponent<HealthComponent>()->IsDead())
	{
		mAcquiredTarget = lastHitActor;
		return true;
	}

	return false;
}

// Check for portal teleport (it returns true if the teleport occurs)
bool TurretHead::PortalTeleport()
{
	// If a turret teleports, it can’t teleport again until 0.25 seconds elapse
	if (mTeleportCooldown > 0.0f)
	{
		return false;
	}

	// You can’t teleport unless there is both a blue and orange portal
	if (mGame->GetBluePortal() != nullptr && mGame->GetOrangePortal() != nullptr)
	{
		// To check for intersection with a portal, you need to use GetParent() to get the
		// parent actor (which is the base of the turret) and then get its CollisionComponent

		Portal* oppositePortal = nullptr;

		if (GetParent()->GetComponent<CollisionComponent>()->Intersect(
				mGame->GetBluePortal()->GetComponent<CollisionComponent>()))
		{
			oppositePortal = mGame->GetOrangePortal();
		}

		if (GetParent()->GetComponent<CollisionComponent>()->Intersect(
				mGame->GetOrangePortal()->GetComponent<CollisionComponent>()))
		{
			oppositePortal = mGame->GetBluePortal();
		}

		// Set the parent’s position to the position of the opposite portal
		if (oppositePortal != nullptr)
		{
			GetParent()->SetPosition(oppositePortal->GetPosition());

			// Add to the fall velocity a vector in the direction of the portal’s quat forward with a magnitude of 250
			mFallVelocity += oppositePortal->GetQuatForward() * 250.0f;

			// If a turret teleports, it can’t teleport again until 0.25 seconds elapse
			mTeleportCooldown = 0.25f;

			return true;
		}
	}

	return false;
}

void TurretHead::Die()
{
	// Change the state to TurretState::Dead
	ChangeState(TurretState::Dead);

	// Set the parent’s quaternion to one rotate about the x-axis by pi/2
	GetParent()->SetQuat(
		Quaternion::Concatenate(GetParent()->GetQuat(), Quaternion(Vector3::UnitX, Math::PiOver2)));

	// Disable the laser component
	mLaserComponent->SetDisabled();
}

void TurretHead::TakeDamage()
{
	// The first first time this function
	if (!mDamageTaken && !GetParent()->GetComponent<HealthComponent>()->IsDead())
	{
		// It should stop the current VO sound, if there is one playing
		if (mGame->GetAudio()->GetSoundState(mCurrentVO) == SoundState::Playing)
		{
			mGame->GetAudio()->StopSound(mCurrentVO);
		}

		// Play "TurretFriendlyFire.ogg" on this actor
		mGame->GetAudio()->PlaySound("TurretFriendlyFire.ogg", false, this);

		mDamageTaken = true;
	}
}