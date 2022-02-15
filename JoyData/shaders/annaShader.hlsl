struct PSInput
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD0;
	float4 clipPos : TEXCOORD1;
};

struct PSOutput
{
	float4 Color: SV_Target;
};

struct MVP
{
	float4x4 model;
	float4x4 view;
	float4x4 projection;
};

Texture2D lightAttachment : register(t1);
ConstantBuffer<MVP> mvp : register(b0);

Texture2D diffuseTexture : register(t1);
Texture2D normalTexture : register(t2);
Texture2D specularTexture : register(t3);

SamplerState g_sampler : register(s0);

inline float4 ComputeNonStereoScreenPos(float4 pos) {
	float4 o = pos * 0.5f;
	o.xy = float2(o.x, o.y * -1) + o.w;
	o.zw = pos.zw;
	return o;
}

PSInput VSMain(float3 position : POSITION, float3 color : COLOR, float3 normal: NORMAL, float2 uv : TEXCOORD)
{
	PSInput result;
	float4x4 resMatrix = mul(mvp.projection, mul(mvp.view, mvp.model));
	result.position = mul(resMatrix, float4(position, 1));
	result.clipPos = ComputeNonStereoScreenPos(result.position);
	//result.clipPos.xy /= result.clipPos.w;
	result.uv = uv;

	return result;
}

PSOutput PSMain(PSInput input) // : SV_TARGET
{
	float4 lightPos = float4(1, 2, 0, 1);
	PSOutput output;
	const float2 screenPosition = (input.clipPos.xy / input.clipPos.w);
	const float4 mainColor = diffuseTexture.Sample(g_sampler, input.uv);
	const float4 light = lightAttachment.Load(float3(input.position.xy, 0));// normalTexture.Sample(g_sampler, screenPosition);// g_texture.Sample(g_sampler, input.uv);

	const float ambient = 0.0f;

	output.Color = mainColor* (ambient + light);

	return output;
}