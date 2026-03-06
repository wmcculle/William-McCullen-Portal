#pragma once
#include "Actor.h"

class EnergyCatcher : public Actor
{
public:
	EnergyCatcher(class Game* game, std::string doorName);
	~EnergyCatcher();

	bool GetActivated() const { return mActivated; }
	void SetActivated();

private:
	class MeshComponent* mMeshComponent;
	class CollisionComponent* mCollisionComponent;

	std::string mDoorName; // which door name this energy catcher is associated with

	bool mActivated = false;
};