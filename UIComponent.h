#pragma once
#include "Math.h"
#include "Component.h"

class UIComponent : public Component
{
protected:
	UIComponent(class Actor* owner);
	friend class Actor;

public:
	~UIComponent() override;

	virtual void Draw(class Shader* shader);

protected:
	// Helper to draw a texture
	static void DrawTexture(const class Shader* shader, const class Texture* texture,
							const Vector2& offset = Vector2::Zero, float scale = 1.0f,
							float angle = 0.0f);
};
