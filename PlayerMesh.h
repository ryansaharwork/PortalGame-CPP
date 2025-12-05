//
// Created by Ryan Sahar on 11/5/25.
//
#pragma once
#include "Actor.h"

class MeshComponent;

class PlayerMesh : public Actor
{
protected:
	PlayerMesh();
	friend class Game;
	void HandleUpdate(float deltaTime) override;

private:
	MeshComponent* mMesh = nullptr;
};