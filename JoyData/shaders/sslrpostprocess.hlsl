#include "common.hlsl"

struct PSInput
{
	float4 position : SV_POSITION;
	float3 clipPos : TEXCOORD0;
	float2 screenPos : TEXCOORD1;
};

ConstantBuffer<MVP> mvp : register(b0);
ConstantBuffer<EngineData> engineData : register(b1);
//ConstantBuffer<SSAOData> ssaoData : register(b2);

Texture2D<float> depthTexture : register(t0);
Texture2D<float4> worldNormalTexture : register(t1);
Texture2D<float4> worldPositionTexture : register(t2);
Texture2D<float4> colorTexture : register(t3);

//SamplerState gsamDepthMap : register(s0);
SamplerState gsamPointClamp : register(s0);
SamplerState gsamLinearWrap : register(s1);


static const float ViewAngleThreshold = 0.1f;
static const float PixelSize = 1.0 / 720.0f;
static const int nNumSteps = 500;

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

inline float3 GetUV(float3 pos)
{
	float3 UVD;
	// Transform the world position to shadow projected space
	float4 posShadowMap = mul(mvp.projection, float4(pos, 1.0));
	// Transform the position to shadow clip space
	UVD = posShadowMap.xyz / posShadowMap.w;
	// Convert to shadow map UV values
	UVD.xy = 0.5 * UVD.xy + 0.5;
	UVD.y = 1.0 - UVD.y;

	return UVD;
}


PSInput VSMain(uint id : SV_VertexID)
{
	PSInput input;
	float2 uv = float2((id << 1) & 2, id & 2);
	input.position = float4(uv * float2(2, -2) + float2(-1, 1), 0, 1);
	input.screenPos = ComputeNonStereoScreenPos(input.position);
	input.clipPos = input.position / input.position.w;

	// Quad covering screen in NDC space.
	//input.screenPos = gTexCoords[id];
	//input.position = float4(2.0f * input.screenPos.x - 1.0f, 1.0f - 2.0f * input.screenPos.y, 0.0f, 1.0f);
	//input.clipPos = input.position / input.position.w;

	return input;
}

float4 PSMain(PSInput input) : SV_Target
{
	const float4 worldPosition = worldPositionTexture.Load(float3(input.position.xy, 0));
	const float4 worldNormal = worldNormalTexture.Load(float3(input.position.xy, 0));


	// Pixel position and normal in view space
	float3 vsPos = mul(mvp.view, worldPosition).xyz;

	float3 vsNorm = normalize(mul(mvp.view, float4(worldNormal.xyz, 0)).xyz);
	// Calculate the camera to pixel direction
	float3 eyeToPixel = normalize(vsPos);
	// Calculate the reflected view direction
	float3 vsReflect = normalize(reflect(eyeToPixel, vsNorm));
	// The initial reflection color for the pixel
	float4 reflectColor = float4(0.0, 0.0, 0.0, 0.0);

	float distance = 10;
	float delta = distance / nNumSteps;
	float L = delta;

	//float rz = depthTexture.SampleLevel(gsamPointClamp, GetUV(vsPos).xy, 0.0f).r;
	//rz = LinearEyeDepth(rz);
	//if (vsPos.z > 5 && vsPos.z < 5.1)
	//{
	//	return float4(1, 1, 1, 1);
	//}
	//else if (rz > 5 && rz < 5.1)
	//{
	//	return float4(1, 0, 0, 1);
	//}
	//else
	//	return float4(0, 0, 0, 0);

	for (uint i = 0; i < nNumSteps; i++)
	{
		float3 newPos = vsPos + vsReflect * L;

		float3 uv = GetUV(newPos);

		//uv.z += 0.001f;

		float rz = depthTexture.SampleLevel(gsamPointClamp, uv.xy, 0.0f).r + 0.001f;
		rz = LinearEyeDepth(rz);

		float d = newPos.z - rz;

		if (abs(d) < 0.02f)
		{
			reflectColor = colorTexture.SampleLevel(gsamLinearWrap, uv.xy, 0.0f);
			//reflectColor.xyz *= (distance - L) / distance;
			break;
		}
		L += delta;
	}

	return reflectColor;
}

float3 GetViewPosition(float2 UV, float depth)
{
	float4 position = 1.0f;
	position.x = UV.x * 2.0f - 1.0f;
	position.y = -(UV.y * 2.0f - 1.0f);
	position.z = depth;
	//Transform Position from Homogenous Space to World Space 
	position = mul(engineData.cameraInvProj, position);
	position /= position.w;
	return position.xyz;
}


//float4 PSMain(PSInput input) : SV_Target
//{
//	const float4 worldPosition = worldPositionTexture.Load(float3(input.position.xy, 0));
//	const float4 worldNormal = worldNormalTexture.Load(float3(input.position.xy, 0));
//
//
//	// Pixel position and normal in view space
//	float3 texelPosition = mul(mvp.view, worldPosition).xyz;
//	float3 vsNorm = normalize(mul(mvp.view, float4(worldNormal.xyz, 0)).xyz);
//
//	float3 currentRay = 0;
//	float3 nuv = 0;
//	float L = 0.1f;
//
//	float3 viewDir = normalize(texelPosition);
//	float3 reflectDir = normalize(reflect(viewDir, vsNorm));
//
//	for (int i = 0; i < 10; i++)
//	{
//		currentRay = texelPosition + reflectDir * L;
//		nuv = GetUV(currentRay); // проецирование позиции на экран
//		float depth = depthTexture.SampleLevel(gsamPointClamp, nuv.xy, 0.0f).r; // чтение глубины из DepthMap по UV
//		float3 newPosition = GetViewPosition(nuv.xy, depth);
//		L = length(texelPosition - newPosition);
//	}
//
//	return colorTexture.SampleLevel(gsamPointClamp, nuv.xy, 0.0f);
//}
