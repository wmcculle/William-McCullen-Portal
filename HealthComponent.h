#pragma once
#include "MeshComponent.h"
#include <functional>

class Actor;

class HealthComponent : public MeshComponent
{
public:
	HealthComponent(Actor* owner, float health = 100.0f);

	void SetOnDamage(std::function<void(const Vector3&)> onDamage) { mOnDamage = onDamage; }
	void SetOnDeath(std::function<void()> onDeath) { mOnDeath = onDeath; }

	float GetHealth() const { return mHealth; }
	bool IsDead() const { return mHealth <= 0.0f; }
	void TakeDamage(float damageAmount, const Vector3& damageLocation);

private:
	float mHealth;

	std::function<void(const Vector3&)> mOnDamage;
	std::function<void()> mOnDeath;
};