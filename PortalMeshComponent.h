#pragma once
#include "MeshComponent.h"

class PortalMeshComponent : public MeshComponent
{
protected:
	PortalMeshComponent(class Actor* owner);
	friend class Actor;

public:
	// Draw this mesh component
	void Draw(class Shader* shader) override;

private:
	class Texture* mMaskTexture = nullptr;
	class Texture* mBlackTexture = nullptr;
};
