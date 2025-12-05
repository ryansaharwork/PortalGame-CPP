//
// Created by Ryan Sahar on 12/3/25.
//
#include "HUD.h"
#include "Shader.h"
#include "Font.h"
#include "Texture.h"
#include "Math.h"
#include "Game.h"
#include "Renderer.h"
#include "Player.h"
#include "HealthComponent.h"

HUD::HUD(Actor* owner)
: UIComponent(owner)
{
	mFont = new Font();
	mFont->Load("Assets/Inconsolata-Regular.ttf");

	mDamageIndicatorTexture =
		gGame.GetRenderer()->GetTexture("Assets/Textures/UI/DamageIndicator.png");
	mDamageOverlayTexture = gGame.GetRenderer()->GetTexture("Assets/Textures/UI/DamageOverlay.png");
}

HUD::~HUD()
{
	if (mSubtitleTexture != nullptr)
	{
		mSubtitleTexture->Unload();
		delete mSubtitleTexture;

		if (mSubtitleShadowTexture != nullptr)
		{
			mSubtitleShadowTexture->Unload();
			delete mSubtitleShadowTexture;
		}
	}

	mFont->Unload();
	delete mFont;
}

void HUD::Draw(Shader* shader)
{
	if (mSubtitleTexture != nullptr)
	{
		float height = static_cast<float>(mSubtitleTexture->GetHeight());
		Vector2 position(0.0f, -325.0f + height / 2.0f);
		if (mSubtitleShadowTexture != nullptr)
		{
			Vector2 shadowOffset(2.0f, -2.0f);
			DrawTexture(shader, mSubtitleShadowTexture, position + shadowOffset);
		}
		DrawTexture(shader, mSubtitleTexture, position);
	}

	if (mDamageIndicatorTime > 0.0f)
	{
		DrawTexture(shader, mDamageIndicatorTexture, Vector2::Zero, 1.0f, mDamageIndicatorAngle);
	}

	Player* player = gGame.GetPlayer();
	if (player)
	{
		HealthComponent* health = player->GetComponent<HealthComponent>();
		if (health && health->IsDead())
		{
			DrawTexture(shader, mDamageOverlayTexture, Vector2::Zero);
		}
	}
}

void HUD::HandleUpdate(float deltaTime)
{
	if (mDamageIndicatorTime > 0.0f)
	{
		mDamageIndicatorTime -= deltaTime;
		if (mDamageIndicatorTime < 0.0f)
		{
			mDamageIndicatorTime = 0.0f;
		}
	}
}

void HUD::PlayerTakeDamage(float angle)
{
	mDamageIndicatorAngle = angle;
	mDamageIndicatorTime = 1.5f;
}

void HUD::ShowSubtitle(const std::string& text)
{
	if (mSubtitleTexture != nullptr)
	{
		mSubtitleTexture->Unload();
		delete mSubtitleTexture;
		mSubtitleTexture = nullptr;
	}

	if (mSubtitleShadowTexture != nullptr)
	{
		mSubtitleShadowTexture->Unload();
		delete mSubtitleShadowTexture;
		mSubtitleShadowTexture = nullptr;
	}

	if (!text.empty())
	{
		std::string subtitleText = "GLaDOS: " + text;
		mSubtitleTexture = mFont->RenderText(subtitleText, Color::LightGreen);
		mSubtitleShadowTexture = mFont->RenderText(subtitleText, Color::Black);
	}
}