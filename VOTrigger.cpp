//
// Created by Ryan Sahar on 12/3/25.
//
#include "VOTrigger.h"
#include "CollisionComponent.h"
#include "Game.h"
#include "Player.h"
#include "AudioSystem.h"
#include "Door.h"
#include "HUD.h"
#include "HealthComponent.h"

VOTrigger::VOTrigger()
{
	mCollision = CreateComponent<CollisionComponent>();
	mCollision->SetSize({1.0f, 1.0f, 1.0f});
	mCurrentSoundHandle = SoundHandle::Invalid;
}

VOTrigger::~VOTrigger() = default;

void VOTrigger::HandleUpdate(float /*deltaTime*/)
{
	Player* player = gGame.GetPlayer();
	HealthComponent* playerHealth = nullptr;
	if (player)
	{
		playerHealth = player->GetComponent<HealthComponent>();
	}

	bool playerIsDead = playerHealth && playerHealth->IsDead();

	if (!mIsActivated)
	{
		if (player && !playerIsDead)
		{
			CollisionComponent* playerColl = player->GetComponent<CollisionComponent>();
			if (playerColl && mCollision->Intersect(playerColl))
			{
				mIsActivated = true;
				PlayNextSound();
			}
		}
	}
	else
	{
		if (playerIsDead)
		{
			if (mCurrentSoundHandle.IsValid() &&
				gGame.GetAudio()->GetSoundState(mCurrentSoundHandle) == SoundState::Playing)
			{
				gGame.GetAudio()->StopSound(mCurrentSoundHandle);
			}
		}
		else
		{
			if (gGame.GetAudio()->GetSoundState(mCurrentSoundHandle) == SoundState::Stopped)
			{
				PlayNextSound();
			}
		}
	}
}

void VOTrigger::HandleInput(const bool keys[], SDL_MouseButtonFlags /*mouseButtons*/,
							const Vector2& /*relativeMouse*/)
{
	const bool F_KEY_NOW = keys[SDL_SCANCODE_F];
	if (F_KEY_NOW && !mPrevF && mIsActivated)
	{
		if (mCurrentSoundHandle.IsValid() &&
			gGame.GetAudio()->GetSoundState(mCurrentSoundHandle) == SoundState::Playing)
		{
			gGame.GetAudio()->StopSound(mCurrentSoundHandle);
			PlayNextSound();
		}
	}
	mPrevF = F_KEY_NOW;
}

void VOTrigger::PlayNextSound()
{
	if (mCurrentSoundIndex < mSounds.size())
	{
		mCurrentSoundHandle = gGame.GetAudio()->PlaySound(mSounds[mCurrentSoundIndex]);
		if (mCurrentSoundIndex < mSubtitles.size())
		{
			Player* player = gGame.GetPlayer();
			if (player && player->GetHUD())
			{
				player->GetHUD()->ShowSubtitle(mSubtitles[mCurrentSoundIndex]);
			}
		}
		mCurrentSoundIndex++;
	}
	else
	{
		Player* player = gGame.GetPlayer();
		if (player && player->GetHUD())
		{
			player->GetHUD()->ShowSubtitle("");
		}

		if (!mDoorName.empty())
		{
			Door* door = gGame.GetDoor(mDoorName);
			if (door)
			{
				door->Open();
			}
		}

		if (!mNextLevel.empty())
		{
			gGame.SetNextLevel(mNextLevel);
		}

		Destroy();
	}
}
