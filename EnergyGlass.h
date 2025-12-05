//
// Created by Ryan Sahar on 11/20/25.
//
#pragma once
#include "Actor.h"

class EnergyGlass : public Actor
{
protected:
	EnergyGlass();
	~EnergyGlass() override;
	friend class Game;
};