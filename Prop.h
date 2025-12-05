//
// Created by Ryan Sahar on 11/4/25.
//
#pragma once
#include "Actor.h"
#include <string>

class MeshComponent;
class Mesh;
class CollisionComponent;

class Prop : public Actor
{
public:
	// Handles mesh and alpha properties
	void SetMesh(const std::string& meshFile, bool usesAlpha);
	// Handles collision property (if we have one)
	void EnableCollision();

protected:
	Prop();
	~Prop() override;
	friend class Game;

private:
	MeshComponent* mMeshComp = nullptr;
	Mesh* mMesh = nullptr;
	CollisionComponent* mColl = nullptr;
};