#include "CommonEngineStructs.h"

struct PSInput
{
	float4 position : SV_POSITION;
	float4 worldNormal: COLOR1;
	float4 worldPosition: COLOR2;
	float2 uv : TEXCOORD0;
};

struct PSOutput
{
	float4 Color: SV_Target;
};

Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);
ConstantBuffer<MVP> mvp : register(b0);
ConstantBuffer<EngineData> data : register(b1);
TextureCube<float4> cubemap : register(t1);


PSInput VSMain(float3 position : POSITION, float3 color : COLOR, float3 normal: NORMAL, float2 uv : TEXCOORD)
{
	PSInput result;
	float4x4 resMatrix = mul(mvp.projection, mul(mvp.view, mvp.model));
	result.position = mul(resMatrix, float4(position, 1));
	result.worldNormal = mul(mvp.model, float4(normal, 0)); // TODO get from gbuffer
	result.worldPosition = mul(mvp.model, float4(position, 1));
	result.uv = uv;

	return result;
}

PSOutput PSMain(PSInput input) // : SV_TARGET
{
	float4 lightPos = float4(1, 2, 0, 1);
	PSOutput output;
	const float4 mainColor = g_texture.Sample(g_sampler, input.uv);


	float3 r = reflect(input.worldPosition.xyz - data.cameraWorldPos, input.worldNormal.xyz);
	float4 reflectionColor = cubemap.Sample(g_sampler, r);
	//const float4 light = lightAttachment.Load(float3(input.position.xy, 0));// normalTexture.Sample(g_sampler, screenPosition);// g_texture.Sample(g_sampler, input.uv);

	const float ambient = 0.0f;

	output.Color = reflectionColor; // *(ambient + light);

	return output;
}
