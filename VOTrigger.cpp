#include "VOTrigger.h"
#include "Game.h"
#include "CollisionComponent.h"
#include "Player.h"
#include "Door.h"
#include "HUD.h"
#include "HealthComponent.h"

VOTrigger::VOTrigger(Game* game, std::string doorName, std::string nextLevel,
					 std::vector<std::string> sounds, std::vector<std::string> subtitles)
: Actor(game)
, mDoorName(doorName)
, mNextLevel(nextLevel)
, mSounds(sounds)
, mSubtitles(subtitles)
{
	mCollisionComponent = new CollisionComponent(this);
	mCollisionComponent->SetSize(1.0f, 1.0f, 1.0f);
}

void VOTrigger::OnUpdate(float deltaTime)
{
	if (mGame->GetPlayer()->GetComponent<HealthComponent>()->IsDead())
	{
		// If the player is dead, you should check to see if a current VO sound is playing, and if it is, stop it
		if (mGame->GetAudio()->GetSoundState(mCurrentSound) == SoundState::Playing)
		{
			mGame->GetAudio()->StopSound(mCurrentSound);
		}

		// Make the existing logic in VOTrigger::OnUpdate only if the player is alive
		return;
	}

	// The first time the VOTrigger intersects with the player, the VOTrigger should activate and play the first sound.
	if (!mActivated &&
		mCollisionComponent->Intersect(mGame->GetPlayer()->GetComponent<CollisionComponent>()))
	{
		mActivated = true;
		PlayNextSound();
	}
	// Once a VOTrigger is activated, on every frame it should check if the sound it’s playing is SoundState::Stopped
	else if (mActivated && mGame->GetAudio()->GetSoundState(mCurrentSound) == SoundState::Stopped)
	{
		// In which case it should start playing the next sound (if there is one).
		if ((mSounds.size() - 1) > mCurrentSoundIndex)
		{
			PlayNextSound();
		}
		// If there are no sounds left to play:
		else
		{
			// If the door name member variable is set to something, open that door
			if (!mDoorName.empty())
			{
				// To “open” a door, for now just remove the door from the collider vector (we’ll add a simple animation for this in the next lab).
				for (Actor* collider : mGame->GetColliders())
				{
					Door* door = dynamic_cast<Door*>(collider);
					if (door != nullptr && door->GetDoorName() == mDoorName)
					{
						mGame->RemoveCollider(door);
						door->SetIsOpen(true);
					}
				}
			}

			// If the next level member variable is set to something, set the next level in Game to it, which will force
			// the level to change to the specified level
			if (!mNextLevel.empty())
			{
				mGame->SetNextLevel(mNextLevel);
			}

			// Make sure that when there are no sounds left, you call ShowSubtitle("") to clear out the texture.
			mGame->GetPlayer()->GetHUD()->ShowSubtitle("");

			// Set the VOTrigger itself to ActorState::Destroy, so it gets removed
			SetState(ActorState::Destroy);
		}
	}
}

void VOTrigger::OnProcessInput(const Uint8* keyState, Uint32 mouseButtons,
							   const Vector2& relativeMouse)
{
	// On a leading edge of the F key, stop the current sound (if the VOTrigger is playing one).
	if (keyState[SDL_SCANCODE_F] && !mFPressed &&
		mGame->GetAudio()->GetSoundState(mCurrentSound) == SoundState::Playing)
	{
		mGame->GetAudio()->StopSound(mCurrentSound);
	}

	mFPressed = keyState[SDL_SCANCODE_F];
}

void VOTrigger::PlayNextSound()
{
	mCurrentSoundIndex++;
	mCurrentSound = mGame->GetAudio()->PlaySound(mSounds[mCurrentSoundIndex]);

	mGame->GetPlayer()->GetHUD()->ShowSubtitle(mSubtitles[mCurrentSoundIndex]);
}