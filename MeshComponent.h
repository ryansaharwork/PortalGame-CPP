#pragma once
#include "Component.h"
#include <cstddef>

class MeshComponent : public Component
{
protected:
	MeshComponent(class Actor* owner, bool usesAlpha = false);
	friend class Actor;

public:
	~MeshComponent() override;
	// Draw this mesh component
	virtual void Draw(class Shader* shader);
	// Set the mesh/texture index used by mesh component
	void SetMesh(class Mesh* mesh) { mMesh = mesh; }
	void SetTextureIndex(size_t index) { mTextureIndex = index; }

protected:
	class Mesh* mMesh;
	size_t mTextureIndex;
	bool mUsesAlpha;
};
