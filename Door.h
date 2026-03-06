#pragma once
#include "Actor.h"

class Door : public Actor
{
public:
	Door(class Game* game, std::string doorName);
	~Door();

	std::string GetDoorName() const { return mDoorName; }
	void SetIsOpen(bool isOpen);

protected:
	void OnUpdate(float deltaTime) override;

private:
	class MeshComponent* mMeshComponent;
	class CollisionComponent* mCollisionComponent;

	std::string mDoorName; // a unique name for that door

	Actor* mDoorLeftHalf;
	Actor* mDoorRightHalf;

	bool mIsOpen = false;
	float mOpenTime = 1.0f;
	float mOpenTimer = 0.0f;
};