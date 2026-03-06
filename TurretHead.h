#pragma once
#include "Actor.h"
#include <map>
#include "AudioSystem.h"

enum class TurretState
{
	Idle,
	Search,
	Priming,
	Firing,
	Falling,
	Dead
};

class TurretHead : public Actor
{
public:
	TurretHead(class Game* game, Actor* parent = nullptr);

	void Die();
	void TakeDamage();

protected:
	void OnUpdate(float deltaTime) override;

private:
	class MeshComponent* mMeshComponent;

	Actor* mLaser;
	class LaserComponent* mLaserComponent;

	TurretState mTurretState = TurretState::Idle;
	float mStateTimer = 0.0f;

	void UpdateIdle(float deltaTime);
	void UpdateSearch(float deltaTime);
	void UpdatePriming(float deltaTime);
	void UpdateFiring(float deltaTime);
	void UpdateFalling(float deltaTime);
	void UpdateDead(float deltaTime);

	void ChangeState(TurretState turretState);

	bool TargetAcquisition();
	Actor* mAcquiredTarget = nullptr;

	void SearchRandomPoint();
	Quaternion mQuatTurretToPoint;

	bool mInterpolateForward = true;
	float mTurnTimer = 0.0f;

	Vector3 mFallVelocity;

	bool PortalTeleport();

	float mTeleportCooldown = 0.0f;

	float mDamageCooldown = 0.05f;

	// Key is the TurretState and the value is the name of the sound file you want to play
	std::map<TurretState, std::string> mSoundMap;
	SoundHandle mCurrentVO;

	bool mDamageTaken = false;
};