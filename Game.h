#pragma once
#include "SDL2/SDL.h"
#include "SDL2/SDL_mixer.h"
#include <unordered_map>
#include <string>
#include <vector>
#include "Math.h"
#include "AudioSystem.h"
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#else
#define emscripten_cancel_main_loop()
#endif

class Player;
class Actor;
class Renderer;
class PortalGun;
class Portal;

class Game
{
public:
	Game();
	bool Initialize();
	void RunLoop();
	void EmRunIteration()
	{
		if (!mIsRunning)
		{
			emscripten_cancel_main_loop();
		}
		ProcessInput();
		UpdateGame();
		GenerateOutput();
	}
	void Shutdown();

	void AddActor(Actor* actor);
	void RemoveActor(Actor* actor);

	AudioSystem* GetAudio() { return mAudio; }

	Renderer* GetRenderer() { return mRenderer; }

	const float WINDOW_WIDTH = 1024.0f;
	const float WINDOW_HEIGHT = 768.0f;

	void SetPlayer(Player* newPlayer) { mPlayer = newPlayer; }
	Player* GetPlayer() const { return mPlayer; }

	void AddCollider(Actor* actor);
	void RemoveCollider(Actor* actor);
	std::vector<Actor*>& GetColliders() { return mColliders; }

	void SetPortalGun(PortalGun* newPortalGun) { mPortalGun = newPortalGun; }
	PortalGun* GetPortalGun() const { return mPortalGun; }

	void SetBluePortal(Portal* bluePortal) { mBluePortal = bluePortal; }
	Portal* GetBluePortal() const { return mBluePortal; }

	void SetOrangePortal(Portal* orangePortal) { mOrangePortal = orangePortal; }
	Portal* GetOrangePortal() const { return mOrangePortal; }

	void ReloadLevel();

	const std::vector<Actor*>& GetActors() const { return mActors; }

	void SetNextLevel(std::string nextLevel) { mNextLevel = nextLevel; }

private:
	void ProcessInput();
	void UpdateGame();
	void GenerateOutput();
	void LoadData();
	void UnloadData();

	// All the actors in the game
	std::vector<Actor*> mActors;

	Renderer* mRenderer = nullptr;
	AudioSystem* mAudio = nullptr;

	Uint32 mTicksCount = 0;
	bool mIsRunning = true;

	Player* mPlayer = nullptr;

	std::vector<Actor*> mColliders;

	PortalGun* mPortalGun = nullptr;

	Portal* mBluePortal = nullptr;
	Portal* mOrangePortal = nullptr;

	std::string mCurrentLevel;
	class InputReplay* mInputReplay = nullptr;

	std::string mNextLevel; // starts out empty

	bool mF5Pressed = false;
};
