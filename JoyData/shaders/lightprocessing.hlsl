#include "common.hlsl"

struct PSInput
{
	float4 position : SV_POSITION;

	//float2 uv : TEXCOORD0;
	//float4 clipPos : TEXCOORD1;
};

struct PSOutput
{
	float4 Color: SV_Target;
};

ConstantBuffer<MVP> mvp : register(b0);
ConstantBuffer<LightData> lightData : register(b1);
ConstantBuffer<EngineData> engineData : register(b2);

Texture2D positionTexture : register(t0);
Texture2D normalTexture : register(t1);

Texture2D shadowMapSpotTexture : register(t2);
TextureCube shadowMapPointTexture : register(t3);

SamplerComparisonState PCFSampler : register(s0);


inline float4 ComputeNonStereoScreenPos(float4 pos)
{
	float4 o = pos * 0.5f;
	o.xy = float2(o.x, o.y * -1) + o.w;
	o.zw = pos.zw;
	return o;
}

static const float PI = 3.14159265f;
static const float ToRad = PI / 180;

float3 ProcessCapsule(float2 uv, float radius, float height)
{
	const float x = uv.x * 2 - 1;
	return float3(
		radius * x + sign(x) * height / 2,
		radius * cos(PI * uv.y * 2) * sin(acos(x)),
		radius * sin(PI * uv.y * 2) * sin(acos(x))
	);
}

float3 ClosestPointToSegment(float3 p, float3 a, float3 b)
{
	const float3 m = b - a;
	const float t = dot(p - a, m) / dot(m, m);
	if (t < 0) return a;
	if (t > 1) return b;

	return a + t * m;
}

float3 ProcessSpot(float2 uv, float angle, float height)
{
	const float xyMultiplier = uv.y == 1 ? 0 : uv.y * height * tan(angle * PI / 360);
	return float3(
		sin(PI * uv.x * 2) * xyMultiplier,
		cos(PI * uv.x * 2) * xyMultiplier,
		uv.y * height
	);
}

float CalcAttenuationByDistance(float distance, float maxDistance)
{
	if (distance > maxDistance) return 0;
	return 1 / (1 + 0.09f * distance + 0.032f * distance * distance);
}

float3 CalcAttenuationForSpot(float3 lightPosition, float3 fragPosition, float3 lightDir, float halfAngleRad, float height)
{
	const float distToLight = length(fragPosition - lightPosition);
	const float3 fragDir = normalize(fragPosition - lightPosition);
	const float cosAng = dot(lightDir, fragDir); // theta
	const float epsilon = cos(halfAngleRad * 0.7) - cos(halfAngleRad);
	if (distToLight > height || cosAng < cos(halfAngleRad)) return 0;


	const float conAtt = saturate((cos(halfAngleRad * 0.7) - cosAng) / epsilon);
	const float distToLightNorm = 1.0 - saturate(distToLight / height);
	return (1 - conAtt); // *distToLightNorm;
}

PSInput VSMain(float3 position : POSITION, float3 color : COLOR, float3 normal : NORMAL, float2 uv : TEXCOORD)
{
	PSInput result;
	const float4x4 resMatrix = mul(mvp.projection, mul(mvp.view, mvp.model));

	if (lightData.angle > 0 && lightData.height > 0)
	{
		position = ProcessSpot(uv, lightData.angle, lightData.height);
	}
	else
	{
		position = ProcessCapsule(uv, lightData.radius, lightData.height);
	}
	result.position = mul(resMatrix, float4(position, 1));

	return result;
}

PSOutput PSMain(PSInput input) // : SV_TARGET
{
	PSOutput output;

	const float3 worldNormal = normalTexture.Load(float3(input.position.xy, 0));
	const float3 worldPos = positionTexture.Load(float3(input.position.xy, 0));

	float3 lightPos;
	float attenuation = 1;

	if (lightData.radius > 0 && lightData.height == 0)
	{
		lightPos = mul(mvp.model, float4(0, 0, 0, 1));
		const float dist = length(lightPos - worldPos);
		attenuation = CalcAttenuationByDistance(dist, lightData.radius);
		float3 ToPixel = worldPos - lightPos;
		float3 ToPixelAbs = abs(ToPixel);
		float Z = max(ToPixelAbs.x, max(ToPixelAbs.y, ToPixelAbs.z));
		float2 LightPerspectiveValues = float2(lightData.projection[2][2], lightData.projection[2][3]);
		float bias = 0.001;

		float Depth = (LightPerspectiveValues.x * Z + LightPerspectiveValues.y) / Z;
		attenuation *= shadowMapPointTexture.SampleCmpLevelZero(PCFSampler, ToPixel, Depth - bias);
	}
	else if (lightData.angle > 0 && lightData.height > 0)
	{
		lightPos = mul(mvp.model, float4(0, 0, 0, 1));
		const float3 lightDir = mul(mvp.model, float4(0, 0, 1, 0));
		attenuation = CalcAttenuationForSpot(
			lightPos,
			worldPos,
			lightDir,
			lightData.angle / 2 * ToRad,
			lightData.height);

		// Transform the world position to shadow projected space
		const float4x4 resMatrix = mul(lightData.projection, lightData.view[0]);
		float4 posShadowMap = mul(resMatrix, float4(worldPos, 1.0));
		// Transform the position to shadow clip space
		float3 UVD = posShadowMap.xyz / posShadowMap.w;
		// Convert to shadow map UV values
		UVD.xy = 0.5 * UVD.xy + 0.5;
		UVD.y = 1.0 - UVD.y;
		float bias = 0.001;
		UVD.z -= bias;
		// Compute the hardware PCF value
		attenuation *= shadowMapSpotTexture.SampleCmpLevelZero(PCFSampler, UVD.xy, UVD.z);
	}
	else if (lightData.radius > 0 && lightData.height > 0)
	{
		const float3 a = mul(mvp.model, float4(-lightData.height / 2, 0, 0, 1));
		const float3 b = mul(mvp.model, float4(+lightData.height / 2, 0, 0, 1));
		lightPos = ClosestPointToSegment(worldPos, a, b);
		const float dist = length(lightPos - worldPos);
		attenuation = CalcAttenuationByDistance(dist, lightData.radius);
	}

	const float3 toLightDir = normalize(lightPos - worldPos);
	const float diff = max(dot(worldNormal, toLightDir), 0.0);

	float3 viewDir = normalize(engineData.cameraWorldPos - worldPos);
	float3 reflectDir = reflect(-toLightDir, worldNormal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);

	output.Color = float4(1, 1, 1, 1) * (diff + spec )* attenuation * lightData.intensity;

	return output;
}
