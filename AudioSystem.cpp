#include "AudioSystem.h"
#include "SDL3/SDL.h"
#include <filesystem>
#include "Game.h"
#include "Player.h"
#include "Actor.h"
#include "Math.h"

SoundHandle SoundHandle::Invalid;

// Create the AudioSystem with specified number of channels
// (Defaults to 8 channels)
AudioSystem::AudioSystem(int numChannels)
{
	// Open audio
	Mix_OpenAudio(0, nullptr);

	// Set # of mixer channels
	Mix_AllocateChannels(numChannels);

	// Resize channel tracking vector to match our channel count
	mChannels.resize(static_cast<size_t>(numChannels), SoundHandle::Invalid);
}

// Destroy the AudioSystem
AudioSystem::~AudioSystem()
{
	// Free all cached sounds
	for (auto& kv : mSounds)
	{
		if (kv.second)
		{
			Mix_FreeChunk(kv.second);
		}
	}
	mSounds.clear();

	// Close audio
	Mix_CloseAudio();
}

// Updates the status of all the active sounds every frame
void AudioSystem::Update(float deltaTime)
{
	Player* player = gGame.GetPlayer();

	for (auto it = mHandleMap.begin(); it != mHandleMap.end();)
	{
		const HandleInfo& hi = it->second;
		const int CH = hi.mChannel;

		if (CH >= 0 && CH < static_cast<int>(mChannels.size()) && Mix_Playing(CH) == 0)
		{
			if (hi.mActor != nullptr)
			{
				auto actorIt = mActorMap.find(hi.mActor);
				if (actorIt != mActorMap.end())
				{
					actorIt->second.erase(it->first);
				}
			}

			mChannels[static_cast<size_t>(CH)].Reset();
			it = mHandleMap.erase(it); // safe: erase returns next iterator
		}
		else
		{
			if (hi.mActor != nullptr && Mix_Playing(CH) != 0)
			{
				int volume = CalculateVolume(hi.mActor, player);
				Mix_Volume(CH, volume);
			}
			++it;
		}
	}
}

// Plays the sound with the specified name and loops if looping is true
// Returns the SoundHandle which is used to perform any other actions on the
// sound when active
SoundHandle AudioSystem::PlaySound(const std::string& soundName, bool looping, class Actor* actor,
								   bool stopOnActorRemove, int fadeTimeMS)
{
	Mix_Chunk* chunk = GetSound(soundName);
	if (!chunk)
	{
		SDL_Log("[AudioSystem] PlaySound couldn't find sound for %s", soundName.c_str());
		return SoundHandle::Invalid;
	}

	// Find first available channel
	int channel = -1;
	for (int i = 0; i < static_cast<int>(mChannels.size()); i++)
	{
		if (!mChannels[i].IsValid())
		{
			channel = i;
			break;
		}
	}

	if (channel == -1)
	{
		SDL_Log("[AudioSystem] No available channel to play sound %s", soundName.c_str());
		return SoundHandle::Invalid;
	}

	int loops = looping ? -1 : 0;
	int result = 0;

	// Play with optional fade-in
	if (fadeTimeMS > 0)
	{
		result = Mix_FadeInChannel(channel, chunk, loops, fadeTimeMS);
	}
	else
	{
		result = Mix_PlayChannel(channel, chunk, loops);
	}

	if (result == -1)
	{
		SDL_Log("[AudioSystem] Failed to play sound %s", soundName.c_str());
		return SoundHandle::Invalid;
	}

	// Set volume based on distance
	Player* player = gGame.GetPlayer();
	int volume = CalculateVolume(actor, player);
	Mix_Volume(channel, volume);

	++mLastHandle;
	mChannels[static_cast<size_t>(channel)] = mLastHandle;

	HandleInfo hi;
	hi.mSoundName = soundName;
	hi.mIsLooping = looping;
	hi.mIsPaused = false;
	hi.mChannel = channel;
	hi.mActor = actor;
	hi.mStopOnActorRemove = stopOnActorRemove;
	mHandleMap.emplace(mLastHandle, hi);

	if (actor != nullptr)
	{
		mActorMap[actor].insert(mLastHandle);
	}

	return mLastHandle;
}

// Stops the sound if playing
void AudioSystem::StopSound(SoundHandle sound, int fadeTimeMS)
{
	auto iter = mHandleMap.find(sound);
	if (iter == mHandleMap.end())
	{
		SDL_Log("[AudioSystem] StopSound couldn't find handle %s", sound.GetDebugStr());
		return;
	}

	const int CH = iter->second.mChannel;
	if (CH >= 0)
	{
		if (fadeTimeMS > 0)
		{
			// Fade out
			Mix_FadeOutChannel(CH, fadeTimeMS);
		}
		else
		{
			// Immediate stop
			Mix_HaltChannel(CH);
			if (CH < static_cast<int>(mChannels.size()))
			{
				mChannels[static_cast<size_t>(CH)].Reset();
			}
			mHandleMap.erase(iter);
		}
	}
}

// Pauses the sound if it is currently playing
void AudioSystem::PauseSound(SoundHandle sound)
{
	auto iter = mHandleMap.find(sound);
	if (iter == mHandleMap.end())
	{
		SDL_Log("[AudioSystem] PauseSound couldn't find handle %s", sound.GetDebugStr());
		return;
	}

	HandleInfo& hi = iter->second;
	if (!hi.mIsPaused)
	{
		if (hi.mChannel >= 0)
		{
			Mix_Pause(hi.mChannel);
		}
		hi.mIsPaused = true;
	}
}

// Resumes the sound if it is currently paused
void AudioSystem::ResumeSound(SoundHandle sound)
{
	auto iter = mHandleMap.find(sound);
	if (iter == mHandleMap.end())
	{
		SDL_Log("[AudioSystem] ResumeSound couldn't find handle %s", sound.GetDebugStr());
		return;
	}

	HandleInfo& hi = iter->second;
	if (hi.mIsPaused)
	{
		if (hi.mChannel >= 0)
		{
			Mix_Resume(hi.mChannel);
		}
		hi.mIsPaused = false;
	}
}

// Returns the current state of the sound
SoundState AudioSystem::GetSoundState(SoundHandle sound) const
{
	auto iter = mHandleMap.find(sound);
	if (iter == mHandleMap.end())
	{
		return SoundState::Stopped;
	}

	const HandleInfo& hi = iter->second;
	if (hi.mIsPaused)
	{
		return SoundState::Paused;
	}
	else
	{
		return SoundState::Playing;
	}
}

// Stops all sounds on all channels
void AudioSystem::StopAllSounds()
{
	Mix_HaltChannel(-1);
	for (auto& mChannel : mChannels)
	{
		mChannel.Reset();
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
void AudioSystem::CacheSound(const std::string& soundName)
{
	GetSound(soundName);
}

// If the sound is already loaded, returns Mix_Chunk from the map.
// Otherwise, will attempt to load the file and save it in the map.
// Returns nullptr if sound is not found.
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
void AudioSystem::ProcessInput(const bool keys[])
{
	// Debugging code that outputs all active sounds on leading edge of period key
	if (keys[SDL_SCANCODE_PERIOD] && !mLastDebugKey)
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

	mLastDebugKey = keys[SDL_SCANCODE_PERIOD];
}

int AudioSystem::CalculateVolume(class Actor* actor, class Actor* listener) const
{
	if (actor == nullptr || listener == nullptr)
	{
		return 128;
	}

	Vector3 actorPos = actor->GetTransform().GetPosition();
	Vector3 listenerPos = listener->GetTransform().GetPosition();
	float distance = (actorPos - listenerPos).Length();

	if (distance >= 600.0f)
	{
		return 0;
	}

	if (distance <= 25.0f)
	{
		return 128;
	}

	// Linear interpolation between 25 and 600 (128 to 0)
	float t = (distance - 25.0f) / (600.0f - 25.0f);
	int volume = static_cast<int>(128.0f * (1.0f - t));
	return volume;
}

void AudioSystem::RemoveActor(class Actor* actor)
{
	auto actorIt = mActorMap.find(actor);
	if (actorIt != mActorMap.end())
	{
		for (const SoundHandle& handle : actorIt->second)
		{
			auto handleIt = mHandleMap.find(handle);
			if (handleIt != mHandleMap.end())
			{
				HandleInfo& hi = handleIt->second;
				hi.mActor = nullptr;

				if (hi.mStopOnActorRemove && Mix_Playing(hi.mChannel) != 0)
				{
					Mix_HaltChannel(hi.mChannel);
				}
			}
		}

		mActorMap.erase(actorIt);
	}
}
