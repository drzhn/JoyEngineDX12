#ifndef JOY_ASSET_HEADER_H
#define JOY_ASSET_HEADER_H

#include <cstdint>

struct MeshAssetHeader
{
	uint32_t vertexDataSize;
	uint32_t indexDataSize;
	uint32_t materialIndex;
};

enum TextureAssetFormat
{
	RGBA8,
	RGBA32,
	BC1_UNORM,
	BC6H_UF16
};

struct TextureAssetHeader
{
	uint32_t width;
	uint32_t height;
	TextureAssetFormat format;
	uint32_t mipCount;
	uint32_t dataSize;
};

#endif // JOY_ASSET_HEADER_H
