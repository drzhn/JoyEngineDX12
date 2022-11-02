#ifndef JOY_ASSET_HEADER_H
#define JOY_ASSET_HEADER_H

struct MeshAssetHeader
{
	uint32_t vertexDataSize;
	uint32_t indexDataSize;
	uint32_t materialIndex;
};

enum TextureAssetFormat
{
	RGBA8,
};

struct TextureAssetHeader
{
	uint32_t width;
	uint32_t height;
	TextureAssetFormat format;
	unsigned char* data;
};

#endif // JOY_ASSET_HEADER_H