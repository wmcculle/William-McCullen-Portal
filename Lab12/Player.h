#pragma once
#include "Actor.h"
#include "AudioSystem.h"

class CollisionComponent;
class PlayerMove;
class CameraComponent;
class HealthComponent;
class HUD;

class Player : public Actor
{
public:
	Player(class Game* game);

	CollisionComponent* GetCollisionComponent() const { return mCollisionComponent; }
	PlayerMove* GetPlayerMove() const { return mPlayerMove; }
	CameraComponent* GetCameraComponent() const { return mCameraComponent; }

	bool HasGun() const { return mHasGun; }
	void GiveGun();

	void SetInitialPos(Vector3 initialPos) { mInitialPos = initialPos; }
	Vector3 GetInitialPos() const { return mInitialPos; }

	HUD* GetHUD() const { return mHUD; }

	SoundHandle GetDeathSound() const { return mDeathSound; }

protected:
	void OnUpdate(float deltaTime) override;

private:
	CollisionComponent* mCollisionComponent;
	PlayerMove* mPlayerMove;
	CameraComponent* mCameraComponent;
	HealthComponent* mHealthComponent;

	bool mHasGun = false;
	void CheckPortalGunCollision();

	Vector3 mInitialPos;

	HUD* mHUD;

	std::vector<std::string> mDeathSounds;
	std::vector<std::string> mDeathSubtitles;
	SoundHandle mDeathSound;
};