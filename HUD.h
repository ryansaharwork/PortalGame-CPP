//
// Created by Ryan Sahar on 12/3/25.
//
#pragma once
#include "UIComponent.h"
#include <string>

class Shader;
class Font;
class Texture;

class HUD : public UIComponent
{
protected:
	HUD(class Actor* owner);
	~HUD() override;
	friend class Actor;

	void Draw(class Shader* shader) override;
	void HandleUpdate(float deltaTime) override;

public:
	void ShowSubtitle(const std::string& text);
	void PlayerTakeDamage(float angle);

private:
	Font* mFont = nullptr;
	Texture* mSubtitleTexture = nullptr;
	Texture* mSubtitleShadowTexture = nullptr;
	Texture* mDamageIndicatorTexture = nullptr;
	Texture* mDamageOverlayTexture = nullptr;
	float mDamageIndicatorAngle = 0.0f;
	float mDamageIndicatorTime = 0.0f;
};