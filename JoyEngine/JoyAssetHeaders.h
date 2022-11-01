#ifndef JOY_ASSET_HEADER_H
#define JOY_ASSET_HEADER_H

struct Vec3
{
	float x;
	float y;
	float z;
};

struct Vec2
{
	float x;
	float y;
};

struct Vertex
{
	Vec3 pos;
	Vec3 color;
	Vec3 normal;
	Vec2 texCoord;
};

enum TextureFormat
{
	RGBA8,
};

struct TextureData
{
	uint32_t width;
	uint32_t height;
	TextureFormat format;
	unsigned char* data;
};

#endif // JOY_ASSET_HEADER_H