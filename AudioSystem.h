#pragma once
#include <unordered_map>
#include <map>
#include <set>
#include <string>
#include <vector>
#include "SDL3_mixer/SDL_mixer.h"

// SoundHandles are used to operate on active sounds
class SoundHandle
{
public:
	// Returns true if this is an active sound handle
	bool IsValid() const { return mID != 0; }

	// Resets to inactive sound handle
	void Reset() { mID = 0; }

	// Increments for convenience
	SoundHandle& operator++()
	{
		mID++;
		return *this;
	}

	SoundHandle operator++(int)
	{
		SoundHandle temp(*this);
		mID++;
		return temp;
	}

	const char* GetDebugStr() const
	{
		static std::string tempStr;
		tempStr = std::to_string(mID);
		return tempStr.c_str();
	}

	// Boolean checks
	bool operator==(const SoundHandle& rhs) const { return mID == rhs.mID; }
	bool operator!=(const SoundHandle& rhs) const { return mID != rhs.mID; }
	bool operator<(const SoundHandle& rhs) const { return mID < rhs.mID; }
	bool operator<=(const SoundHandle& rhs) const { return mID <= rhs.mID; }
	bool operator>(const SoundHandle& rhs) const { return mID > rhs.mID; }
	bool operator>=(const SoundHandle& rhs) const { return mID >= rhs.mID; }

	static SoundHandle Invalid;

private:
	unsigned int mID = 0;
};

// Used to get information about state of sound
enum class SoundState
{
	Stopped,
	Playing,
	Paused
};

// Manages playing audio through SDL_mixer
class AudioSystem
{
public:
	// Create the AudioSystem with specified number of channels
	// (Defaults to 8 channels)
	AudioSystem(int numChannels = 8);
	// Destroy the AudioSystem
	~AudioSystem();

	// Updates the status of all the active sounds every frame
	void Update(float deltaTime);
	// Input for debugging purposes
	void ProcessInput(const bool keys[]);

	// Plays the sound with the specified name and loops if looping is true
	// Returns the SoundHandle which is used to perform any other actions on the
	// sound when active
	// NOTE: The soundName is without the "Assets/Sounds/" part of the file
	//       For example, pass in "ChompLoop.wav" rather than
	//       "Assets/Sounds/ChompLoop.wav".
	SoundHandle PlaySound(const std::string& soundName, bool looping = false,
						  class Actor* actor = nullptr, bool stopOnActorRemove = true,
						  int fadeTimeMS = 0);

	// Stops the sound if it is currently playing
	void StopSound(SoundHandle sound, int fadeTimeMS = 0);

	// Pauses the sound if it is currently playing
	void PauseSound(SoundHandle sound);

	// Resumes the sound if it is currently paused
	void ResumeSound(SoundHandle sound);

	// Returns the current state of the sound
	SoundState GetSoundState(SoundHandle sound) const;

	// Stops all sounds on all channels
	void StopAllSounds();

	// Removes an actor from the audio system, stopping associated sounds
	void RemoveActor(class Actor* actor);

	// Cache all sounds under Assets/Sounds
	void CacheAllSounds();

	// Used to preload the sound data of a sound
	// NOTE: The soundName is without the "Assets/Sounds/" part of the file
	//       For example, pass in "ChompLoop.wav" rather than
	//       "Assets/Sounds/ChompLoop.wav".
	void CacheSound(const std::string& soundName);

private:
	int CalculateVolume(class Actor* actor, class Actor* listener) const;
	// If the sound is already loaded, returns Mix_Chunk from the map.
	// Otherwise, will attempt to load the file and save it in the map.
	// Returns nullptr if sound is not found.
	// NOTE: The soundName is without the "Assets/Sounds/" part of the file
	//       For example, pass in "ChompLoop.wav" rather than
	//       "Assets/Sounds/ChompLoop.wav".
	Mix_Chunk* GetSound(const std::string& soundName);

	// Internal struct used to track active sound handles properties
	struct HandleInfo
	{
		std::string mSoundName;
		int mChannel = -1;
		bool mIsLooping = false;
		bool mIsPaused = false;
		class Actor* mActor = nullptr;
		bool mStopOnActorRemove = true;
	};

	// Tracks the active SoundHandle for each channel
	std::vector<SoundHandle> mChannels;

	// Maps all the active SoundHandles to their HandleInfo
	std::map<SoundHandle, HandleInfo> mHandleMap;

	// Map for actors to their handles
	std::unordered_map<class Actor*, std::set<SoundHandle>> mActorMap;

	// Map to store the Mix_Chunk data for all the files
	std::unordered_map<std::string, Mix_Chunk*> mSounds;

	// Used to track the last audio handle value used
	SoundHandle mLastHandle;

	// Debug input in ProcessInput
	bool mLastDebugKey = false;
};
