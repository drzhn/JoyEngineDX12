#include "CommonEngineStructs.h"

struct _uint
{
	uint data;
};

ConstantBuffer<MVP> mvp : register(b0);
ConstantBuffer<_uint> isHorizontal : register(b1);
ConstantBuffer<SSAOData> ssaoData : register(b2);

Texture2D depthTexture : register(t0);
Texture2D viewNormalTexture : register(t1);
Texture2D gInputMap : register(t2);

SamplerState gsamDepthMap : register(s0);
SamplerState gsamLinearWrap : register(s1);
SamplerState gsamPointClamp : register(s2);

static const int gBlurRadius = 5;

static const float2 gTexCoords[6] =
{
	float2(0.0f, 1.0f),
	float2(0.0f, 0.0f),
	float2(1.0f, 0.0f),
	float2(0.0f, 1.0f),
	float2(1.0f, 0.0f),
	float2(1.0f, 1.0f)
};

struct VertexOut
{
	float4 PosH : SV_POSITION;
	float2 TexC : TEXCOORD;
};

VertexOut VSMain(uint vid : SV_VertexID)
{
	VertexOut vout;

	vout.TexC = gTexCoords[vid];

	// Quad covering screen in NDC space.
	vout.PosH = float4(2.0f * vout.TexC.x - 1.0f, 1.0f - 2.0f * vout.TexC.y, 0.0f, 1.0f);

	return vout;
}

float NdcDepthToViewDepth(float z_ndc)
{
	// z_ndc = A + B/viewZ, where gProj[2,2]=A and gProj[3,2]=B.
	float viewZ = mvp.projection[2][3] / (z_ndc - mvp.projection[2][2]);
	return viewZ;
}

float PSMain(VertexOut pin) : SV_Target
{
	// unpack into float array.
	float blurWeights[12] =
	{
		ssaoData.blurWeights[0].x, ssaoData.blurWeights[0].y, ssaoData.blurWeights[0].z, ssaoData.blurWeights[0].w,
		ssaoData.blurWeights[1].x, ssaoData.blurWeights[1].y, ssaoData.blurWeights[1].z, ssaoData.blurWeights[1].w,
		ssaoData.blurWeights[2].x, ssaoData.blurWeights[2].y, ssaoData.blurWeights[2].z, ssaoData.blurWeights[2].w,
	};

	float2 texOffset;
	if (isHorizontal.data == 1)
	{
		texOffset = float2(ssaoData.invRenderTargetSize.x, 0.0f);
	}
	else
	{
		texOffset = float2(0.0f, ssaoData.invRenderTargetSize.y);
	}

	// The center value always contributes to the sum.
	float color = blurWeights[gBlurRadius] * gInputMap.SampleLevel(gsamPointClamp, pin.TexC, 0.0).r;
	float totalWeight = blurWeights[gBlurRadius];

	float3 centerNormal = viewNormalTexture.SampleLevel(gsamPointClamp, pin.TexC, 0.0f).xyz;
	float centerDepth = NdcDepthToViewDepth(
		depthTexture.SampleLevel(gsamDepthMap, pin.TexC, 0.0f).r);

	for (float i = -gBlurRadius; i <= gBlurRadius; ++i)
	{
		// We already added in the center weight.
		if (i == 0)
			continue;

		float2 tex = pin.TexC + i * texOffset;

		float3 neighborNormal = viewNormalTexture.SampleLevel(gsamPointClamp, tex, 0.0f).xyz;
		float neighborDepth = NdcDepthToViewDepth(depthTexture.SampleLevel(gsamDepthMap, tex, 0.0f).r);

		//
		// If the center value and neighbor values differ too much (either in 
		// normal or depth), then we assume we are sampling across a discontinuity.
		// We discard such samples from the blur.
		//

		if (dot(neighborNormal, centerNormal) >= 0.8f &&
			abs(neighborDepth - centerDepth) <= 0.2f)
		{
			float weight = blurWeights[i + gBlurRadius];

			// Add neighbor pixel to blur.
			color += weight * gInputMap.SampleLevel(
				gsamPointClamp, tex, 0.0).r;

			totalWeight += weight;
		}
	}

	// Compensate for discarded samples by making total weights sum to 1.
	return color / totalWeight;
}
