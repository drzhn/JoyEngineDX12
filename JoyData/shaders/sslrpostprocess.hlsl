#include "common.hlsl"

struct PSInput
{
	float4 position : SV_POSITION;
	float4 screenPos : POSITION;
	float3 clipPos : TEXCOORD0;
};

ConstantBuffer<MVP> mvp : register(b0);
//ConstantBuffer<EngineData> engineData : register(b1);
//ConstantBuffer<SSAOData> ssaoData : register(b2);

Texture2D<float> depthTexture : register(t0);
Texture2D<float4> viewNormalTexture : register(t1);
Texture2D<float4> worldPositionTexture : register(t2);
Texture2D<float4> colorTexture : register(t3);

//SamplerState gsamDepthMap : register(s0);
//SamplerState gsamLinearWrap : register(s1);
SamplerState gsamPointClamp : register(s0);


static const float ViewAngleThreshold = 0.8f;
static const float PixelSize = 2.0 / 768.0f;
static const int nNumSteps = 1024;

inline float LinearEyeDepth(float depth)
{
	// z_ndc = A + B/viewZ, where gProj[2,2]=A and gProj[3,2]=B.
	float viewZ = mvp.projection[2][3] / (depth - mvp.projection[2][2]);
	return viewZ;
}

inline float4 ComputeNonStereoScreenPos(float4 pos)
{
	float4 o = pos * 0.5f;
	o.xy = float2(o.x, o.y * -1) + o.w;
	o.zw = pos.zw;
	return o;
}


PSInput VSMain(uint id : SV_VertexID)
{
	PSInput input;
	float2 uv = float2((id << 1) & 2, id & 2);
	input.position = float4(uv * float2(2, -2) + float2(-1, 1), 0, 1);
	input.screenPos = ComputeNonStereoScreenPos(input.position);
	input.clipPos = input.position / input.position.w;
	return input;
}

float4 PSMain(PSInput input) : SV_Target
{
	const float4 worldPosition = worldPositionTexture.Load(float3(input.position.xy, 0));
	const float4 viewNormal = viewNormalTexture.Load(float3(input.position.xy, 0));

	// Pixel position and normal in view space
	float3 vsPos = mul(mvp.view, worldPosition).xyz;
	float3 vsNorm = normalize(viewNormal).xyz;
	// Calculate the camera to pixel direction
	float3 eyeToPixel = normalize(vsPos);
	// Calculate the reflected view direction
	float3 vsReflect = reflect(eyeToPixel, vsNorm);
	// The initial reflection color for the pixel
	float4 reflectColor = float4(0.0, 0.0, 0.0, 0.0);
	// Don't bother with reflected vector above the threshold vector
	if (vsReflect.z > ViewAngleThreshold)
	{
		// Fade the reflection as the view angles gets close to the threshold
		float viewAngleThresholdInv = 1.0 - ViewAngleThreshold;
		float viewAngleFade = (vsReflect.z - ViewAngleThreshold) / viewAngleThresholdInv;
		// Transform the View Space Reflection to clip-space
		float3 vsPosReflect = vsPos + vsReflect;
		float3 csPosReflect = mul(mvp.projection, float4(vsPosReflect, 1.0)).xyz / vsPosReflect.z;
		float3 csReflect = csPosReflect - input.clipPos;
		// Resize Screen Space Reflection to an appropriate length.
		float reflectScale = PixelSize / length(csReflect.xy);
		csReflect *= reflectScale;
		// Calculate the offsets in texture space
		float3 currOffset = input.clipPos + csReflect;
		currOffset.xy = currOffset.xy * float2(0.5, -0.5) + 0.5;
		float3 lastOffset = input.clipPos;
		lastOffset.xy = lastOffset.xy * float2(0.5, -0.5) + 0.5;
		csReflect = float3(csReflect.x * 0.5, csReflect.y * -0.5,
		                   csReflect.z);
		// Iterate over the HDR texture searching for intersection
		for (int nCurStep = 0; nCurStep < nNumSteps; nCurStep++)
		{
			// Sample from depth buffer
			float curSample = depthTexture.SampleLevel(gsamPointClamp, currOffset.xy, 0.0).x + 0.005f;
			if (curSample < currOffset.z)
			{
				// Correct the offset based on the sampled depth
				currOffset.xy = lastOffset.xy + (currOffset.z - curSample) *
					csReflect.xy;
				// Get the HDR value at the location
				reflectColor.xyz = colorTexture.SampleLevel(gsamPointClamp,
				                                      currOffset.xy, 0.0).xyz;
				//// Fade out samples close to the texture edges
				//float edgeFade = saturate(distance(currOffset.xy,
				//                                   float2(0.5, 0.5)) * 2.0 - EdgeDistThreshold);
				//// Find the fade value
				//reflectColor.w = min(viewAngleFade, 1.0 - edgeFade * edgeFade);
				// Apply the reflection sacle
				reflectColor.w *= reflectScale;
				// Advance past the final iteration to break the loop
				nCurStep = nNumSteps;
			}
			// Advance to the next sample
			lastOffset = currOffset;
			currOffset += csReflect;
		}
	}
	return reflectColor;
}
