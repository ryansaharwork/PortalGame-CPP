#pragma once
#include "SDL3/SDL.h"
#include <vector>
#include <string>
#include <unordered_map>
#include "AudioSystem.h"
#include "InputReplay.h"

class Player;
class Portal;
class Door;
class EnergyCatcher;

class Game
{
public:
	bool Initialize();
	bool RunIteration();
	void Shutdown();
	void HandleEvent(const SDL_Event* event);

	template <typename T>
	T* CreateActor()
	{
		T* actor = new T();
		mPendingCreate.emplace_back(actor);
		return actor;
	}

	void AddPendingDestroy(class Actor* actor);

	AudioSystem* GetAudio() const { return mAudio; }
	class Renderer* GetRenderer() const { return mRenderer; }
	InputReplay* GetInputReplay() const { return mInputReplay; }

	std::vector<class Actor*>& GetActors() { return mActors; }

	Player* GetPlayer() const { return mPlayer; }
	void SetPlayer(Player* player) { mPlayer = player; }

	void AddCollider(Actor* actor);
	void RemoveCollider(Actor* actor);
	std::vector<Actor*>& GetColliders() { return mColliders; }

	class Portal* GetBluePortal() const { return mBluePortal; }
	class Portal* GetOrangePortal() const { return mOrangePortal; }

	void SetBluePortal(class Portal* p) { mBluePortal = p; }
	void SetOrangePortal(class Portal* p) { mOrangePortal = p; }

	Door* GetDoor(const std::string& name) const;
	void RegisterDoor(const std::string& name, Door* door);

	void OnEnergyCatcherActivated(EnergyCatcher* catcher);

	const std::string& GetCurrentLevel() const { return mCurrentLevel; }
	void SetNextLevel(const std::string& level) { mNextLevel = level; }

private:
	void ProcessInput();
	void UpdateGame();
	void GenerateOutput() const;
	void LoadData();
	void UnloadData();
	void DestroyActor(Actor* actor);

	// TUNABLE CONSTANTS
	static constexpr int AUDIO_CHANNELS = 32;
	static constexpr float FIXED_DELTA_TIME = 0.016f; // 60 FPS
	static constexpr float WINDOW_WIDTH = 1024.0f;
	static constexpr float WINDOW_HEIGHT = 768.0f;

	// Projection
	static constexpr float CAMERA_FOV = 1.22f;
	static constexpr float CAMERA_NEAR = 10.0f;
	static constexpr float CAMERA_FAR = 10000.0f;

	// All the actors in the game
	std::vector<Actor*> mActors;
	std::vector<Actor*> mPendingCreate;
	std::vector<Actor*> mPendingDestroy;
	std::vector<Actor*> mColliders;

	class Renderer* mRenderer = nullptr;
	AudioSystem* mAudio = nullptr;

	Uint64 mTicksCount = 0;
	bool mIsRunning = true;

	Player* mPlayer = nullptr;

	// Track blue/orange portals
	Portal* mBluePortal = nullptr;
	Portal* mOrangePortal = nullptr;

	std::string mCurrentLevel;
	std::string mNextLevel;
	InputReplay* mInputReplay = nullptr;

	std::unordered_map<std::string, Door*> mDoorsByName;

	bool mPrevF5 = false;
};

extern Game gGame;
