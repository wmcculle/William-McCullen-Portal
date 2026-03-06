#include "HealthComponent.h"
#include "Shader.h"
#include "Mesh.h"
#include "Actor.h"
#include "Game.h"
#include "Renderer.h"

HealthComponent::HealthComponent(Actor* owner, float health)
: MeshComponent(owner, true)
, mHealth(health)
{
	SetMesh(GetGame()->GetRenderer()->GetMesh("Assets/Meshes/Laser.gpmesh"));
}

void HealthComponent::TakeDamage(float damageAmount, const Vector3& damageLocation)
{
	// Don’t do anything if already dead
	if (!IsDead())
	{
		// Subtract the damage amount from the health
		mHealth -= damageAmount;

		// Call the mOnDamage callback if it’s set (passing in the location as a parameter)
		if (mOnDamage)
		{
			mOnDamage(damageLocation);
		}

		if (IsDead())
		{
			if (mOnDeath)
			{
				mOnDeath();
			}
		}
	}
}