#include "Game.h"
#include <algorithm>
#include "Actor.h"
#include <fstream>
#include "Renderer.h"
#include "InputReplay.h"
#include "LevelLoader.h"
#include "Random.h"
#include <SDL3_ttf/SDL_ttf.h>

Game gGame;

bool Game::Initialize()
{
	// Request 60 FPS
	SDL_SetHint("SDL_MAIN_CALLBACK_RATE", "60");

	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
	{
		SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
		return false;
	}

	mRenderer = new Renderer(this);
	if (!mRenderer->Initialize(WINDOW_WIDTH, WINDOW_HEIGHT))
	{
		SDL_Log("Failed to start renderer");
		return false;
	}

	mAudio = new AudioSystem(AUDIO_CHANNELS);
	TTF_Init();

	Random::Init();
	mInputReplay = new InputReplay(this);

	LoadData();
	mTicksCount = SDL_GetTicks();

	// Mac: control-click = right click
	SDL_SetHint(SDL_HINT_MAC_CTRL_CLICK_EMULATE_RIGHT_CLICK, "1");

	// Enable relative mouse mode
	SDL_SetWindowRelativeMouseMode(mRenderer->GetWindow(), true);
	SDL_GetRelativeMouseState(nullptr, nullptr); // Clear previous relative data

	return true;
}

bool Game::RunIteration()
{
	if (!mIsRunning)
		return false;

	ProcessInput();
	UpdateGame();
	GenerateOutput();

	return true;
}

void Game::AddCollider(Actor* actor)
{
	mColliders.emplace_back(actor);
}

void Game::RemoveCollider(Actor* actor)
{
	std::erase(mColliders, actor);
}

Door* Game::GetDoor(const std::string& name) const
{
	auto it = mDoorsByName.find(name);
	if (it != mDoorsByName.end())
	{
		return it->second;
	}
	return nullptr;
}
void Game::RegisterDoor(const std::string& name, Door* door)
{
	mDoorsByName[name] = door;
}

void Game::ProcessInput()
{
	const bool* state = SDL_GetKeyboardState(nullptr);

#ifndef __EMSCRIPTEN__
	if (state[SDL_SCANCODE_ESCAPE])
	{
		mIsRunning = false;
	}
#endif

	// PLAYBACK ON P
	if (state[SDL_SCANCODE_P])
	{
		mInputReplay->StartPlayback(mCurrentLevel, false);
	}

	// F5 to reload level (leading edge) - but not during replay playback
	bool f5Now = state[SDL_SCANCODE_F5];
	if (f5Now && !mPrevF5 && !mInputReplay->IsInPlayback())
	{
		mNextLevel = mCurrentLevel;
	}
	mPrevF5 = f5Now;

	float x = 0.0f;
	float y = 0.0f;
	SDL_MouseButtonFlags mouseButtons = SDL_GetRelativeMouseState(&x, &y);
	Vector2 relativeMouse(x, y);

	// Let replay override inputs if in playback
	mInputReplay->InputPlayback(state, mouseButtons, relativeMouse);

	for (Actor* actor : mActors)
	{
		actor->Input(state, mouseButtons, relativeMouse);
	}

	mAudio->ProcessInput(state);
}

void Game::UpdateGame()
{
#ifdef __EMSCRIPTEN__
	Uint64 tickNow = SDL_GetTicks();
	float waitTime = (tickNow - mTicksCount) / 1000.0f;

	// Wait until 60 FPS interval reached
	while (waitTime < FIXED_DELTA_TIME)
	{
		tickNow = SDL_GetTicks();
		waitTime = (tickNow - mTicksCount) / 1000.0f;
	}
#endif

	float deltaTime = FIXED_DELTA_TIME; // Force 60 FPS update

	mTicksCount = SDL_GetTicks();

	mAudio->Update(deltaTime);

	for (auto actor : mPendingCreate)
		mActors.emplace_back(actor);
	mPendingCreate.clear();

	mInputReplay->Update(deltaTime);

	for (auto actor : mActors)
		actor->Update(deltaTime);

	for (auto actor : mPendingDestroy)
		DestroyActor(actor);
	mPendingDestroy.clear();

	// Check if we need to reload/load a level
	if (!mNextLevel.empty())
	{
		// STEP 1: Call UnloadData()
		UnloadData();

		// STEP 2: Clear collider vector and door map
		mColliders.clear();
		mDoorsByName.clear();

		// STEP 3: Call StopPlayback() on input replay
		mInputReplay->StopPlayback();

		// STEP 4: Call StopAllSounds() on audio system
		mAudio->StopAllSounds();

		// STEP 5: Set both portal pointers to null
		mBluePortal = nullptr;
		mOrangePortal = nullptr;

		// STEP 6: Set current level string to next level string
		mCurrentLevel = mNextLevel;

		// STEP 7: Call LevelLoader::Load with new level name
		LevelLoader::Load(mCurrentLevel);

		// STEP 8: Clear next level string
		mNextLevel.clear();
	}
}

void Game::GenerateOutput() const
{
	mRenderer->Draw();
}

void Game::LoadData()
{
	// Precache all the sounds (do not remove)
	mAudio->CacheAllSounds();

	// Projection with no magic numbers
	Matrix4 proj = Matrix4::CreatePerspectiveFOV(CAMERA_FOV, WINDOW_WIDTH, WINDOW_HEIGHT,
												 CAMERA_NEAR, CAMERA_FAR);

	Matrix4 view = Matrix4::CreateLookAt(Vector3::Zero, Vector3::UnitX, Vector3::UnitZ);

	mRenderer->SetProjectionMatrix(proj);
	mRenderer->SetViewMatrix(view);

	mCurrentLevel = "Assets/Level01.json";
	LevelLoader::Load(mCurrentLevel);
}

void Game::UnloadData()
{
	for (auto actor : mActors)
		delete actor;

	mActors.clear();
}

void Game::DestroyActor(Actor* actor)
{
	std::erase(mActors, actor);
	delete actor;
}

void Game::Shutdown()
{
	if (mInputReplay)
	{
		delete mInputReplay;
		mInputReplay = nullptr;
	}

	UnloadData();

	delete mAudio;
	mRenderer->Shutdown();
	delete mRenderer;

	SDL_Quit();
}

void Game::HandleEvent(const SDL_Event* event)
{
	switch (event->type)
	{
	case SDL_EVENT_QUIT:
		mIsRunning = false;
		break;
	default:
		break;
	}
}

void Game::AddPendingDestroy(class Actor* actor)
{
	if (std::ranges::find(mPendingDestroy, actor) == mPendingDestroy.end())
	{
		mPendingDestroy.emplace_back(actor);
	}
}
