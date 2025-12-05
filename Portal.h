//
// Created by Ryan Sahar on 10/30/25.
//
#pragma once
#include "Actor.h"

class Portal : public Actor
{
public:
	void Setup(const Vector3& pos, const Vector3& normal, bool isBlue);
	Vector3 GetPortalOutVector(const Vector3& inVec, const Portal* exitPortal, float w) const;
	bool IsBlue() const { return mIsBlue; }
	void HandleUpdate(float deltaTime) override;

protected:
	Portal();
	friend class Game;

private:
	bool mIsBlue = false; // Track if portal is blue
	void CalcViewMatrix(struct PortalData& portalData, Portal* exitPortal) const;
};