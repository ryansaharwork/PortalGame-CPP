#pragma once
#include <string>
#include <SDL3/SDL_surface.h>

class Texture
{
public:
	Texture();

	bool Load(const std::string& fileName);
	void Unload() const;
	void CreateFromSurface(const SDL_Surface* surface);
	void CreateForRendering(int width, int height, unsigned int format);

	void SetActive(int index = 0) const;

	int GetWidth() const { return mWidth; }
	int GetHeight() const { return mHeight; }

	unsigned int GetTextureID() const { return mTextureID; }

private:
	unsigned int mTextureID;
	int mWidth;
	int mHeight;
};
