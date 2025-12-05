//
// Created by Ryan Sahar on 12/3/25.
//
#pragma once
#include "Actor.h"
#include "AudioSystem.h"
#include <string>
#include <vector>

class CollisionComponent;
class Player;

class VOTrigger : public Actor
{
protected:
	VOTrigger();
	~VOTrigger() override;
	friend class Game;

	void HandleUpdate(float deltaTime) override;
	void HandleInput(const bool keys[], SDL_MouseButtonFlags mouseButtons,
					 const Vector2& relativeMouse) override;

public:
	void SetDoorName(const std::string& doorName) { mDoorName = doorName; }
	void SetNextLevel(const std::string& nextLevel) { mNextLevel = nextLevel; }
	void SetSounds(const std::vector<std::string>& sounds) { mSounds = sounds; }
	void SetSubtitles(const std::vector<std::string>& subtitles) { mSubtitles = subtitles; }

private:
	void PlayNextSound();

	CollisionComponent* mCollision = nullptr;
	std::string mDoorName;
	std::string mNextLevel;
	std::vector<std::string> mSounds;
	std::vector<std::string> mSubtitles;

	bool mIsActivated = false;
	size_t mCurrentSoundIndex = 0;
	SoundHandle mCurrentSoundHandle;

	bool mPrevF = false;
};
