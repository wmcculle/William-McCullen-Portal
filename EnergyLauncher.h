#pragma once
#include "Actor.h"

class EnergyLauncher : public Actor
{
public:
	EnergyLauncher(class Game* game, float pelletCooldown, std::string doorName);
	~EnergyLauncher();

	std::string GetDoorName() const { return mDoorName; }
	void DisableShoot() { mCanShoot = false; }

protected:
	void OnUpdate(float deltaTime) override;

private:
	class MeshComponent* mMeshComponent;
	class CollisionComponent* mCollisionComponent;

	float mPelletCooldown; // specifies how frequently the energy launcher shoots a pellet
	std::string mDoorName; // which door name this energy launcher is associated with

	float mPelletCooldownTimer = 0.0f;

	bool mCanShoot = true;
};