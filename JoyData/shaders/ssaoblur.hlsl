#include "common.hlsl"

struct PSInput
{
	float4 PosH : SV_POSITION;
	float2 TexC : TEXCOORD0;
	float3 PosV : POSITION;
};

struct PSOutput
{
	float4 Color: SV_Target;
};

ConstantBuffer<EngineData> data : register(b0);
Texture2D<float> depthTexture : register(t0);
Texture2D<float4> viewNormalTexture : register(t1);

static const float2 gTexCoords[6] =
{
	float2(0.0f, 1.0f),
	float2(0.0f, 0.0f),
	float2(1.0f, 0.0f),

	float2(0.0f, 1.0f),
	float2(1.0f, 0.0f),
	float2(1.0f, 1.0f)
};

inline float LinearEyeDepth(float depth)
{
	const float zNear = 0.1f;
	const float zFar = 1000.0f;
	float x = 1 - zFar / zNear;
	float y = zFar / zNear;
	float z = x / zFar;
	float w = y / zFar;
	return 1.0 / (z * depth + w);
}

float ComputeFog(float z)
{
	float fog = 0.0;
	fog = (30 - z) / (30 - 10);
	//fog = exp2(-0.1f * z);
	return saturate(fog);
}

PSInput VSMain(uint id : SV_VertexID)
{
	PSInput vout;
	vout.TexC = gTexCoords[id];
	// Quad covering screen in NDC space.
	vout.PosH = float4(2.0f * vout.TexC.x - 1.0f, 1.0f -
	                   2.0f * vout.TexC.y, 0.0f, 1.0f);

	// Transform quad corners to view space near plane.
	float4 ph = mul(data.cameraInvProj, vout.PosH);
	vout.PosV = ph.xyz / ph.w;

	return vout;
}

PSOutput PSMain(PSInput v)
{
	PSOutput output;

	const float4 viewNormal = viewNormalTexture.Load(float3(v.PosH.xy, 0));
	const float depth = depthTexture.Load(float3(v.PosH.xy, 0));


	// Get z-coord of this pixel in NDC space from depth
	// Transform depth to view space.
	float pz = LinearEyeDepth(depth);
	// Reconstruct the view space position of the point	with depth pz.
	float3 p = (pz / v.PosV.z) * v.PosV;

	return output;
}
