#include "CommonEngineStructs.h"

struct GS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float4 WorldPos : COLOR1;
	uint RTIndex : SV_RenderTargetArrayIndex;
};

ConstantBuffer<CubemapConvolutionConstants> constants : register(b0);

TextureCube<float4> environmentMap : register(t0);

SamplerState g_sampler : register(s0);

static const float PI = 3.14159265359;

float4 VSMain(float3 position : POSITION) : SV_POSITION
{
	return float4(position, 1);
}

[maxvertexcount(18)]
void GSMain(triangle float4 InPos[3] : SV_Position, inout TriangleStream<GS_OUTPUT> OutStream)
{
	for (int iFace = 0; iFace < 6; iFace++)
	{
		GS_OUTPUT output;
		output.RTIndex = iFace;
		for (int v = 0; v < 3; v++)
		{
			output.WorldPos = mul(constants.model, InPos[v]);
			output.Pos = mul(constants.projection, mul(constants.view[iFace], output.WorldPos));
			OutStream.Append(output);
		}
		OutStream.RestartStrip();
	}
}

float4 PSMain(GS_OUTPUT input) : SV_TARGET
{
	// The world vector acts as the normal of a tangent surface
	// from the origin, aligned to WorldPos. Given this normal, calculate all
	// incoming radiance of the environment. The result of this radiance
	// is the radiance of light coming from -Normal direction, which is what
	// we use in the PBR shader to sample irradiance.
	float3 N = normalize(input.WorldPos);

	float3 irradiance = float3(0, 0, 0);

	// tangent space calculation from origin point
	float3 up = float3(0.0, 1.0, 0.0);
	float3 right = normalize(cross(up, N));
	up = normalize(cross(N, right));

	float sampleDelta = 0.1;
	float nrSamples = 0.0;
	for (float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
	{
		for (float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
		{
			// spherical to cartesian (in tangent space)
			float3 tangentSample = float3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
			// tangent space to world
			float3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N;

			irradiance += environmentMap.Sample(g_sampler, sampleVec).rgb * cos(theta) * sin(theta);
			nrSamples++;
		}
	}
	irradiance = PI * irradiance * (1.0 / float(nrSamples));

	return float4(irradiance, 1.0);
	//return float4(1,0,0, 1.0);
}
