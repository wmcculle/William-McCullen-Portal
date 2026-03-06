#pragma once
#include "Actor.h"
#include "AudioSystem.h"

class VOTrigger : public Actor
{
public:
	VOTrigger(class Game* game, std::string doorName, std::string nextLevel,
			  std::vector<std::string> sounds, std::vector<std::string> subtitles);

protected:
	void OnUpdate(float deltaTime) override;
	void OnProcessInput(const Uint8* keyState, Uint32 mouseButtons,
						const Vector2& relativeMouse) override;

private:
	class CollisionComponent* mCollisionComponent;

	std::string mDoorName;
	std::string mNextLevel;
	std::vector<std::string> mSounds;
	std::vector<std::string> mSubtitles;

	bool mActivated = false;
	SoundHandle mCurrentSound;
	int mCurrentSoundIndex = -1;

	void PlayNextSound();

	bool mFPressed = false;
};