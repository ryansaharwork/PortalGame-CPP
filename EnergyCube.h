//
// Created by Ryan Sahar on 11/20/25.
//
#pragma once
#include "Actor.h"

class EnergyCube : public Actor
{
protected:
	EnergyCube();
	~EnergyCube() override;
	friend class Game;
};