#include "AudioSystem.h"
#include "SDL2/SDL.h"
#include "SDL2/SDL_mixer.h"
#include <filesystem>
#include "Game.h"
#include "Actor.h"
#include "Player.h"

SoundHandle SoundHandle::Invalid;

// Create the AudioSystem with specified number of channels
// (Defaults to 8 channels)
AudioSystem::AudioSystem(Game* game, int numChannels)
: mGame(game)
{
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);

	Mix_AllocateChannels(numChannels);

	mChannels.resize(numChannels);
}

// Destroy the AudioSystem
AudioSystem::~AudioSystem()
{
	for (auto& sound : mSounds)
	{
		if (sound.second != nullptr)
		{
			Mix_FreeChunk(sound.second);
		}
	}

	mSounds.clear();

	Mix_CloseAudio();
}

// Updates the status of all the active sounds every frame
void AudioSystem::Update(float deltaTime)
{
	// Every frame, you need to do a regular for loop over mChannels
	for (int i = 0; i < mChannels.size(); i++)
	{
		// For any indices which have an IsValid() SoundHandle
		if (mChannels[i].IsValid())
		{
			// Use Mix_Playing to see if that sound is still playing on its corresponding SDL channel number.
			if (!Mix_Playing(i))
			{
				// Before you call mHandleMap.erase, you should check if the SoundHandle has an actor associated with it
				// (which you can find out from the HandleInfo for it). If it does have an actor associated with it, you
				// should erase the SoundHandle from the actor’s corresponding set.
				auto handleMapIterator = mHandleMap.find(mChannels[i]);
				if (handleMapIterator != mHandleMap.end())
				{
					if (handleMapIterator->second.mActor != nullptr)
					{
						mActorMap[handleMapIterator->second.mActor].erase(mChannels[i]);
					}
				}

				// If the sound is NOT playing anymore
				// Remove that SoundHandle from the mHandleMap
				mHandleMap.erase(mChannels[i]);

				// Reset that index in mChannels with .Reset() as the channel should be flagged as available again.
				mChannels[i].Reset();
			}
			else
			{
				auto handleMapIterator = mHandleMap.find(mChannels[i]);
				if (handleMapIterator != mHandleMap.end())
				{
					if (handleMapIterator->second.mActor != nullptr)
					{
						int volume = CalculateVolume(handleMapIterator->second.mActor,
													 mGame->GetPlayer());
						Mix_Volume(i, volume);
					}
				}
			}
		}
	}
}

// Plays the sound with the specified name and loops if looping is true
// Returns the SoundHandle which is used to perform any other actions on the
// sound when active
// NOTE: The soundName is without the "Assets/Sounds/" part of the file
//       For example, pass in "ChompLoop.wav" rather than
//       "Assets/Sounds/ChompLoop.wav".
SoundHandle AudioSystem::PlaySound(const std::string& soundName, bool looping, Actor* actor,
								   bool stopOnActorRemove, int fadeTimeMS)
{
	int selectedChannel = -1;

	// Use GetSound to get the Mix_Chunk* for the sound.
	Mix_Chunk* sound = GetSound(soundName);

	// If the sound cannot be loaded
	if (sound == nullptr)
	{
		SDL_Log("[AudioSystem] PlaySound couldn't find sound for %s", soundName.c_str());

		// Return SoundHandle::Invalid to signify that PlaySound failed.
		return SoundHandle::Invalid;
	}

	// Using a normal for loop, find the first SDL_mixer channel in mChannels that’s available
	// This is the channel you will want to play this new sound on.
	for (int i = 0; i < mChannels.size(); i++)
	{
		if (!mChannels[i].IsValid())
		{
			// Save that channel as the channel to play the new sound on
			selectedChannel = i;
			break;
		}
	}

	// If none of the channels are available, then you need to select which sound to overwrite
	// Use the following prioritization:

	// The oldest instance of the same soundName
	if (selectedChannel == -1)
	{
		for (const auto& handle : mHandleMap)
		{
			if (handle.second.mSoundName == soundName)
			{
				// Save that channel as the channel to play the new sound on
				selectedChannel = handle.second.mChannel;

				// Once you decide to overwrite a sound, you should output an error message
				SDL_Log("[AudioSystem] PlaySound ran out of channels playing %s! Stopping %s",
						soundName.c_str(), handle.second.mSoundName.c_str());

				// Erase the old sound’s SoundHandle from mHandleMap
				mHandleMap.erase(handle.first);

				break;
			}
		}
	}

	// The oldest non-looping sound and if there is none…
	if (selectedChannel == -1)
	{
		for (const auto& handle : mHandleMap)
		{
			if (!handle.second.mIsLooping)
			{
				// Save that channel as the channel to play the new sound on
				selectedChannel = handle.second.mChannel;

				// Once you decide to overwrite a sound, you should output an error message
				SDL_Log("[AudioSystem] PlaySound ran out of channels playing %s! Stopping %s",
						soundName.c_str(), handle.second.mSoundName.c_str());

				// Erase the old sound’s SoundHandle from mHandleMap
				mHandleMap.erase(handle.first);

				break;
			}
		}
	}

	// The oldest sound
	if (selectedChannel == -1)
	{
		// Save that channel as the channel to play the new sound on
		selectedChannel = mHandleMap.begin()->second.mChannel;

		// Once you decide to overwrite a sound, you should output an error message
		SDL_Log("[AudioSystem] PlaySound ran out of channels playing %s! Stopping %s",
				soundName.c_str(), mHandleMap.begin()->second.mSoundName.c_str());

		// Erase the old sound’s SoundHandle from mHandleMap
		mHandleMap.erase(mHandleMap.begin()->first);
	}

	// Once the channel is selected:

	// Make the new sound’s unique SoundHandle
	mLastHandle++;

	// Setup a HandleInfo for the new sound you’re about to play
	HandleInfo handleInfo = {soundName, selectedChannel, looping, false, actor, stopOnActorRemove};

	// Add to mHandleMap the SoundHandle and HandleInfo pair
	mHandleMap.insert({mLastHandle, handleInfo});

	// Furthermore, if actor is not nullptr, you also want to add the SoundHandle for the new sound you’re
	// playing to the mActorMap’s set for that actor.
	if (actor != nullptr)
	{
		mActorMap[actor].insert(mLastHandle);
	}

	// Update the index you selected in mChannels to have the SoundHandle for this new sound
	mChannels[selectedChannel] = mLastHandle;

	if (fadeTimeMS > 0)
	{
		if (looping)
		{
			Mix_FadeInChannel(selectedChannel, sound, -1, fadeTimeMS);
		}
		else
		{
			Mix_FadeInChannel(selectedChannel, sound, 0, fadeTimeMS);
		}
	}
	// Play the sound using Mix_PlayChannel:
	// The third parameter should be -1 if looping is true or otherwise 0.
	else if (looping)
	{
		Mix_PlayChannel(selectedChannel, sound, -1);
	}
	else
	{
		Mix_PlayChannel(selectedChannel, sound, 0);
	}

	int volume = CalculateVolume(actor, mGame->GetPlayer());
	Mix_Volume(selectedChannel, volume);

	// Return the sound handle for the new sound.
	return mLastHandle;
}

// Stops the sound if it is currently playing
void AudioSystem::StopSound(SoundHandle sound, int fadeTimeMS)
{
	// If the SoundHandle is not in mHandleMap, you should output a log error message like this:
	auto it = mHandleMap.find(sound);
	if (it == mHandleMap.end())
	{
		SDL_Log("[AudioSystem] StopSound couldn't find handle %s", sound.GetDebugStr());
	}
	// If it’s in the map
	else
	{
		if (fadeTimeMS > 0)
		{
			Mix_FadeOutChannel(it->second.mChannel, fadeTimeMS);
		}
		else
		{
			// Call Mix_HaltChannel on the appropriate channel,
			Mix_HaltChannel(it->second.mChannel);
			// Reset() that index in the mChannels
			mChannels[it->second.mChannel].Reset();
			// Remove the SoundHandle from mHandleMap.
			mHandleMap.erase(sound);
		}
	}
}

// Pauses the sound if it is currently playing
void AudioSystem::PauseSound(SoundHandle sound)
{
	// If the SoundHandle is not in mHandleMap, you should output a log error message like this:
	auto it = mHandleMap.find(sound);
	if (it == mHandleMap.end())
	{
		SDL_Log("[AudioSystem] PauseSound couldn't find handle %s", sound.GetDebugStr());
	}
	// If the handle is in the map
	// Check the value of mIsPaused in the handle map to make sure you
	// aren’t trying to pause a sound that is already paused.
	else if (!it->second.mIsPaused)
	{
		// Then, call either Mix_Pause (for PauseSound).
		Mix_Pause(it->second.mChannel);

		// Update mIsPaused in the handle map to the new value.
		it->second.mIsPaused = true;
	}
}

// Resumes the sound if it is currently paused
void AudioSystem::ResumeSound(SoundHandle sound)
{
	// If the SoundHandle is not in mHandleMap, you should output a log error message like this:
	auto it = mHandleMap.find(sound);
	if (it == mHandleMap.end())
	{
		SDL_Log("[AudioSystem] ResumeSound couldn't find handle %s", sound.GetDebugStr());
	}
	// If the handle is in the map
	// Check the value of mIsPaused in the handle map to make sure you
	// aren’t trying to resume a sound that’s already resumed.
	else if (it->second.mIsPaused)
	{
		// Then, call  Mix_Resume (for ResumeSound).
		Mix_Resume(it->second.mChannel);

		// Update mIsPaused in the handle map to the new value.
		it->second.mIsPaused = false;
	}
}

// Returns the current state of the sound
SoundState AudioSystem::GetSoundState(SoundHandle sound)
{
	// If the SoundHandle is not in the mHandleMap, this function returns SoundState::Stopped.
	auto it = mHandleMap.find(sound);
	if (it == mHandleMap.end())
	{
		return SoundState::Stopped;
	}
	else
	{
		// Otherwise, it should return either SoundState::Paused or SoundState::Playing
		// depending on the value of the HandleInfo’s mIsPaused element
		if (it->second.mIsPaused)
		{
			return SoundState::Paused;
		}
		else
		{
			return SoundState::Playing;
		}
	}
}

// Stops all sounds on all channels
void AudioSystem::StopAllSounds()
{
	// Stop ALL sounds.
	Mix_HaltChannel(-1);

	for (int i = 0; i < mChannels.size(); i++)
	{
		mChannels[i].Reset();
	}

	mHandleMap.clear();
}

// Cache all sounds under Assets/Sounds
void AudioSystem::CacheAllSounds()
{
#ifndef __clang_analyzer__
	std::error_code ec{};
	for (const auto& rootDirEntry : std::filesystem::directory_iterator{"Assets/Sounds", ec})
	{
		std::string extension = rootDirEntry.path().extension().string();
		if (extension == ".ogg" || extension == ".wav")
		{
			std::string fileName = rootDirEntry.path().stem().string();
			fileName += extension;
			CacheSound(fileName);
		}
	}
#endif
}

// Used to preload the sound data of a sound
// NOTE: The soundName is without the "Assets/Sounds/" part of the file
//       For example, pass in "ChompLoop.wav" rather than
//       "Assets/Sounds/ChompLoop.wav".
void AudioSystem::CacheSound(const std::string& soundName)
{
	GetSound(soundName);
}

// If the sound is already loaded, returns Mix_Chunk from the map.
// Otherwise, will attempt to load the file and save it in the map.
// Returns nullptr if sound is not found.
// NOTE: The soundName is without the "Assets/Sounds/" part of the file
//       For example, pass in "ChompLoop.wav" rather than
//       "Assets/Sounds/ChompLoop.wav".
Mix_Chunk* AudioSystem::GetSound(const std::string& soundName)
{
	std::string fileName = "Assets/Sounds/";
	fileName += soundName;

	Mix_Chunk* chunk = nullptr;
	auto iter = mSounds.find(fileName);
	if (iter != mSounds.end())
	{
		chunk = iter->second;
	}
	else
	{
		chunk = Mix_LoadWAV(fileName.c_str());
		if (!chunk)
		{
			SDL_Log("[AudioSystem] Failed to load sound file %s", fileName.c_str());
			return nullptr;
		}

		mSounds.emplace(fileName, chunk);
	}
	return chunk;
}

// Input for debugging purposes
void AudioSystem::ProcessInput(const Uint8* keyState)
{
	// Debugging code that outputs all active sounds on leading edge of period key
	if (keyState[SDL_SCANCODE_PERIOD] && !mLastDebugKey)
	{
		SDL_Log("[AudioSystem] Active Sounds:");
		for (size_t i = 0; i < mChannels.size(); i++)
		{
			if (mChannels[i].IsValid())
			{
				auto iter = mHandleMap.find(mChannels[i]);
				if (iter != mHandleMap.end())
				{
					HandleInfo& hi = iter->second;
					SDL_Log("Channel %d: %s, %s, looping = %d, paused = %d",
							static_cast<unsigned>(i), mChannels[i].GetDebugStr(),
							hi.mSoundName.c_str(), hi.mIsLooping, hi.mIsPaused);
				}
				else
				{
					SDL_Log("Channel %d: %s INVALID", static_cast<unsigned>(i),
							mChannels[i].GetDebugStr());
				}
			}
		}
	}

	mLastDebugKey = keyState[SDL_SCANCODE_PERIOD];
}

// The purpose of this function is that we’ll call it whenever an actor is destroyed, in which case
// RemoveActor will see if there are any sounds associated with the actor which now need to be halted.
void AudioSystem::RemoveActor(class Actor* actor)
{
	// Try to find the actor in mActorMap (not all actors will be in the map). If the actor is in the map:
	auto actorMapIterator = mActorMap.find(actor);
	if (actorMapIterator != mActorMap.end())
	{
		// Iterate over each handle in the set for that actor
		for (SoundHandle handle : actorMapIterator->second)
		{
			// Try to find the handle in the mHandleMap (not all handles may be here, if they’ve already
			// stopped). If the handle is in the mHandleMap:
			auto handleMapIterator = mHandleMap.find(handle);
			if (handleMapIterator != mHandleMap.end())
			{
				// Set its corresponding handle info’s mActor to nullptr
				handleMapIterator->second.mActor = nullptr;

				// If the handle info has mStopOnActorRemove and Mix_Playing says the sound is playing
				if (handleMapIterator->second.mStopOnActorRemove &&
					Mix_Playing(handleMapIterator->second.mChannel))
				{
					// Call Mix_HaltChannel on the sound
					Mix_HaltChannel(handleMapIterator->second.mChannel);
				}
			}
		}

		// Erase the actor from the map
		mActorMap.erase(actor);
	}
}

// This function returns a volume from 0 to 128 based on the distance between the actors and a
// inner/outer radius function:
int AudioSystem::CalculateVolume(class Actor* actor, class Actor* listener) const
{
	// If either actor or listener is nullptr, just return 128.
	if (actor == nullptr || listener == nullptr)
	{
		return 128;
	}

	// For the position of the actors you MUST use GetWorldPosition() since there may be actor parenting
	// involved, and then calculate the distance between these world positions
	float distance = Vector3::Distance(actor->GetWorldPosition(), listener->GetWorldPosition());

	// If the distance is >= 600.0f, CalculateVolume should return 0
	if (distance >= mVolumeOuterRadius)
	{
		return 0;
	}

	// If the distance is <= 25.0f, CalculateVolume should return 128
	if (distance <= mVolumeInnerRadius)
	{
		return 128;
	}

	// For any distance between 25 to 600, you should set the volume as a linear function from volume
	// of 128 to volume of 0.
	float volume = 128.0f * (1.0f - (distance - mVolumeInnerRadius) /
										(mVolumeOuterRadius - mVolumeInnerRadius));

	return static_cast<int>(volume);
}