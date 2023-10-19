#include "CommonEngineStructs.h"

struct PSInput
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD0;
	float4 clipPos : TEXCOORD1;
	float4 worldNormal : COLOR1;
	float4 worldPos : COLOR2;
};

struct PSOutput
{
	float4 Color: SV_Target;
};

ConstantBuffer<MVP> mvp : register(b0);
ConstantBuffer<EngineData> engineData : register(b1);

Texture2D lightAttachment : register(t0);

Texture2D diffuseTexture : register(t1);
Texture2D normalTexture : register(t2);
Texture2D specularTexture : register(t3);
Texture2D roughnessTexture : register(t4);
TextureCube environmentTexture : register(t5);

SamplerState g_sampler : register(s0);

static const float PI = 3.14159265359;
static const float3 lightPositions[4] = {
	float3(0, 0, -3),
	float3(0, 2, -3),
	float3(2, 0, -3),
	float3(2, 2, -3)
};

static const float3 lightColors[4] = {
	float3(1, 1, 1),
	float3(1, 1, 1),
	float3(1, 1, 1),
	float3(1, 1, 1)
};

inline float4 ComputeNonStereoScreenPos(float4 pos)
{
	float4 o = pos * 0.5f;
	o.xy = float2(o.x, o.y * -1) + o.w;
	o.zw = pos.zw;
	return o;
}

float DistributionGGX(float3 N, float3 H, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH * NdotH;

	float num = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;

	return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r * r) / 8.0;

	float num = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return num / denom;
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx2 = GeometrySchlickGGX(NdotV, roughness);
	float ggx1 = GeometrySchlickGGX(NdotL, roughness);

	return ggx1 * ggx2;
}

float3 fresnelSchlick(float cosTheta, float3 F0)
{
	return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

static const float2 invAtan = float2(0.1591, 0.3183);

float2 SampleSphericalMap(float3 v)
{
	//float2 uv = float2(atan2(v.z, v.x), asin(v.y));
	//uv *= invAtan;
	//uv += 0.5;

	float r = length(v);
	float theta = acos(-v.y / r);
	float phi = atan2(v.x, -v.z);
	float2 uv = float2(
		0.5 + phi / 6.2831852,
		theta / 3.1415926
	);
	return uv;
}


PSInput VSMain(float3 position : POSITION,  float3 normal: NORMAL, float2 uv : TEXCOORD)
{
	PSInput result;
	float4x4 resMatrix = mul(mvp.projection, mul(mvp.view, mvp.model));
	result.position = mul(resMatrix, float4(position, 1));
	result.worldNormal = mul(mvp.model, float4(normal, 0));
	result.worldPos = mul(mvp.model, float4(position, 1));
	result.clipPos = ComputeNonStereoScreenPos(result.position);
	//result.clipPos.xy /= result.clipPos.w;
	result.uv = uv;

	return result;
}

PSOutput PSMain(PSInput input) // : SV_TARGET
{
	PSOutput output;
	const float2 screenPosition = (input.clipPos.xy / input.clipPos.w);
	const float4 diffuse = diffuseTexture.Sample(g_sampler, input.uv);
	const float4 normal = normalTexture.Sample(g_sampler, input.uv);
	const float4 specular = specularTexture.Sample(g_sampler, input.uv);
	const float4 roughness = roughnessTexture.Sample(g_sampler, input.uv);
	const float4 light = lightAttachment.Load(float3(input.position.xy, 0)); // normalTexture.Sample(g_sampler, screenPosition);// g_texture.Sample(g_sampler, input.uv);

	//float2 uv = SampleSphericalMap(normalize(-input.worldNormal.xyz)); // make sure to normalize localPos
	float4 irradiance = environmentTexture.Sample(g_sampler, input.worldNormal);

	float metallic = 0.9f;


	float3 N = normalize(input.worldNormal);
	float3 V = normalize(engineData.cameraWorldPos - input.worldPos);

	float3 F0 = float3(1, 1, 1) * 0.04f;
	F0 = clamp(F0, diffuse, metallic);

	// reflectance equation
	float3 Lo = float3(0, 0, 0);

	for (int i = 0; i < 4; ++i)
	{
		// calculate per-light radiance
		float3 L = normalize(lightPositions[i] - input.worldPos);
		float3 H = normalize(V + L);
		float distance = length(lightPositions[i] - input.worldPos);
		float attenuation = 1.0 / (distance * distance);
		float3 radiance = lightColors[i] * attenuation;

		// cook-torrance brdf
		float NDF = DistributionGGX(N, H, roughness);
		float G = GeometrySmith(N, V, L, roughness);
		float3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

		float3 kS = F;
		float3 kD = float3(1, 1, 1) - kS;
		kD *= 1.0 - metallic;

		float3 numerator = NDF * G * F;
		float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
		float3 spec = numerator / denominator; // *specular;

		// add to outgoing radiance Lo
		float NdotL = max(dot(N, L), 0.0);
		Lo += (kD * diffuse / PI + spec) * radiance * NdotL;
	}

	float3 ambient = irradiance * diffuse;
	float3 color = ambient + Lo;

	color = color / (color + float3(1, 1, 1));
	color = pow(color, float3(1, 1, 1) / 2.2);

	output.Color = float4(color, 1);

	return output;
}
