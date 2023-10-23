#ifndef JOY_ASSET_HEADER_H
#define JOY_ASSET_HEADER_H

#include <cstdint>
#include <Common/Math/MathTypes.h>

namespace JoyEngine
{
	struct TreeEntry
	{
		uint64_t nameHash;
		uint32_t childCount;
		uint32_t childStartIndex;
		uint64_t dataFileOffset;
	};

	struct MeshAssetHeader
	{
		uint32_t vertexDataSize =  0;
		uint32_t indexDataSize = 0;
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
}

#endif // JOY_ASSET_HEADER_H
