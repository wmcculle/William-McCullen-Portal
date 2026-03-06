#include "Pellet.h"
#include "Game.h"
#include "MeshComponent.h"
#include "Renderer.h"
#include "Mesh.h"
#include "CollisionComponent.h"
#include "MoveComponent.h"
#include "Player.h"
#include "Portal.h"
#include "EnergyCatcher.h"
#include "EnergyCube.h"
#include "EnergyGlass.h"
#include "HealthComponent.h"

Pellet::Pellet(Game* game)
: Actor(game)
{
	mMeshComponent = new MeshComponent(this, true);
	mMeshComponent->SetMesh(game->GetRenderer()->GetMesh("Assets/Meshes/Sphere.gpmesh"));
	mMeshComponent->SetTextureIndex(1);

	mCollisionComponent = new CollisionComponent(this);
	mCollisionComponent->SetSize(25.0f, 25.0f, 25.0f);
}

void Pellet::OnUpdate(float deltaTime)
{
	// Update forward movement
	Vector3 forwardVelocity = GetQuatForward() * mPelletSpeed;
	SetPosition(GetPosition() + forwardVelocity * deltaTime);

	// If the Pellet intersects with the player, it should be destroyed.
	if (mCollisionComponent->Intersect(mGame->GetPlayer()->GetCollisionComponent()))
	{
		// Call TakeDamage on the player’s HealthComponent, passing in 100 for the damage and the pellet’s position as the location
		mGame->GetPlayer()->GetComponent<HealthComponent>()->TakeDamage(100.0f, GetPosition());

		Destroy();
	}

	// For its first 0.25 seconds of existence, Pellet should not collide with colliders
	if (mInvincibilityTimer > 0.0f)
	{
		mInvincibilityTimer -= deltaTime;
	}
	else
	{
		// Pellets should also teleport through portals
		// the pellet can only teleport if there’s both an blue and orange portal
		if (GetGame()->GetBluePortal() != nullptr && GetGame()->GetOrangePortal() != nullptr)
		{
			Portal* exitPortal = nullptr;
			if (mCollisionComponent->Intersect(
					mGame->GetBluePortal()->GetComponent<CollisionComponent>()))
			{
				exitPortal = GetGame()->GetOrangePortal();
			}
			else if (mCollisionComponent->Intersect(
						 mGame->GetOrangePortal()->GetComponent<CollisionComponent>()))
			{
				exitPortal = GetGame()->GetBluePortal();
			}

			if (exitPortal != nullptr)
			{
				// When the pellet teleports, it should move to a position 10 units in front of the exit portal.
				// Keep in mind that portals use quaternions, so their forward direction is GetQuatForward().
				SetPosition(exitPortal->GetPosition() + exitPortal->GetQuatForward() * 10.0f);

				// It should also change so that its current movement direction is in the direction the exit portal faces.
				SetQuat(exitPortal->GetQuat());

				// for 0.25 seconds after teleporting, the pellet should ignore any collisions with portals or colliders
				mInvincibilityTimer = 0.25f;
				// This includes ignoring any further portal/collider collisions on the frame the teleport occurs.
				return;
			}
		}

		// intersecting with a collider should destroy the pellet
		for (Actor* collider : mGame->GetColliders())
		{
			if (mCollisionComponent->Intersect(collider->GetComponent<CollisionComponent>()))
			{
				EnergyCatcher* energyCatcher = dynamic_cast<EnergyCatcher*>(collider);
				EnergyCube* energyCube = dynamic_cast<EnergyCube*>(collider);
				EnergyGlass* energyGlass = dynamic_cast<EnergyGlass*>(collider);
				HealthComponent* healthComponent = collider->GetComponent<HealthComponent>();

				// if the collider is an EnergyCatcher and the energy catcher has not already activated,
				// the energy catcher should “catch” the pellet
				if (energyCatcher != nullptr && !energyCatcher->GetActivated())
				{
					// The pellet should be positioned to be 40 units in front of the energy catcher (EnergyCatcher does not use quaternions for rotation)
					SetPosition(energyCatcher->GetPosition() + energyCatcher->GetForward() * 40.0f);
					// The pellet should no longer move
					mPelletSpeed = 0.0f;
					// The EnergyCatcher should become “activated”
					energyCatcher->SetActivated();
				}
				// tests if the collider the pellet is intersecting with is an EnergyCube
				else if (energyCube != nullptr)
				{
					// Turn green (set the mesh component TextureIndex to 2)
					mMeshComponent->SetTextureIndex(2);
				}
				// Similarly add logic for the pellet colliding with EnergyGlass:
				else if (energyGlass != nullptr)
				{
					// If it’s green, it should go through the energy glas
					// Otherwise, it should die
					if (mMeshComponent->GetTextureIndex() != 2)
					{
						Destroy();
					}
				}
				// When detecting a collision against a collider, if the collider has a HealthComponent
				// If it’s dead, ignore the collision (this is so the pellet doesn’t keep hitting a turret that’s dead)
				else if (healthComponent != nullptr)
				{
					if (!healthComponent->IsDead())
					{
						// Deal 100 damage with the pellet’s position as the location
						healthComponent->TakeDamage(100.0f, GetPosition());
						Destroy();
					}
					else
					{
						return;
					}
				}
				else
				{
					Destroy();
				}
			}
		}
	}
}

void Pellet::Destroy()
{
	mGame->GetAudio()->PlaySound("EnergyCaught.ogg", false, this, false);
	SetState(ActorState::Destroy);
}