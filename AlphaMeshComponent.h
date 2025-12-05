//
// Created by Sanjay on 10/24/2025.
//
#pragma once
#include "MeshComponent.h"

class AlphaMeshComponent : public MeshComponent
{
protected:
	AlphaMeshComponent(class Actor* owner);
	friend class Actor;
};
