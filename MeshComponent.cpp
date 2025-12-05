#include "MeshComponent.h"
#include "Shader.h"
#include "Mesh.h"
#include "Actor.h"
#include "Game.h"
#include "Renderer.h"
#include "Texture.h"
#include "VertexArray.h"

MeshComponent::MeshComponent(Actor* owner, bool usesAlpha)
: Component(owner)
, mMesh(nullptr)
, mTextureIndex(0)
, mUsesAlpha(usesAlpha)
{
	gGame.GetRenderer()->AddMeshComp(this, mUsesAlpha);
}

MeshComponent::~MeshComponent()
{
	gGame.GetRenderer()->RemoveMeshComp(this, mUsesAlpha);
}

void MeshComponent::Draw(Shader* shader)
{
	if (mMesh)
	{
		// Set the world transform
		shader->SetMatrixUniform("uWorldTransform", mOwner->GetTransform().GetWorldTransform());
		// Set the active texture
		Texture* t = mMesh->GetTexture(mTextureIndex);
		if (t)
		{
			t->SetActive();
		}
		// Set the mesh's vertex array as active
		VertexArray* va = mMesh->GetVertexArray();
		va->SetActive();
		// Draw
		glDrawElements(GL_TRIANGLES, static_cast<int>(va->GetNumIndices()), GL_UNSIGNED_INT,
					   nullptr);
	}
}
