//
// Created by Ryan Sahar on 11/4/25.
//
#pragma once
#include "Actor.h"

class CollisionComponent;
class MeshComponent;

class Block : public Actor
{
protected:
	Block();
	~Block() override;
	friend class Game;

private:
	MeshComponent* mMesh = nullptr;
	CollisionComponent* mColl = nullptr;
};