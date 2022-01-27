struct PSInput
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD0;
	//float4 clipPos : TEXCOORD1;
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

//Texture2D g_texture : register(t0);
//SamplerState g_sampler : register(s0);
ConstantBuffer<MVP> mvp : register(b0);
//Texture2D positionTexture : register(t1);
//Texture2D normalTexture : register(t2);

//inline float4 ComputeNonStereoScreenPos(float4 pos) {
//	float4 o = pos * 0.5f;
//	o.xy = float2(o.x, o.y * -1) + o.w;
//	o.zw = pos.zw;
//	return o;
//}

static const float PI = 3.14159265f;

float3 ProcessSphere(float2 uv, float radius)
{
	float x = uv.x * 2 - 1;

	return float3(
		radius * x,
		radius * cos(PI * uv.y * 2) * sin(acos(x)),
		radius * sin(PI * uv.y * 2) * sin(acos(x))
	);
}

float3 ProcessCapsule(float2 uv, float radius, float height)
{
	float x = uv.x * 2 - 1;
	float xPos = (height + 1) * x;

	return float3(
		x > 0 ? x + height / 2 : x - height / 2,
		radius * cos(PI * uv.y * 2) * sin(acos(x)),
		radius * sin(PI * uv.y * 2) * sin(acos(x))
	);
}

PSInput VSMain(float3 position : POSITION, float3 color : COLOR, float3 normal : NORMAL, float2 uv : TEXCOORD)
{
	PSInput result;
	float4x4 resMatrix = mul(mvp.projection, mul(mvp.view, mvp.model));

	//position = ProcessSphere(uv, 1);
	position = ProcessCapsule(uv, 1, 2);

	result.position = mul(resMatrix, float4(position, 1));
	//result.clipPos = ComputeNonStereoScreenPos(result.position);
	//result.clipPos.xy /= result.clipPos.w;
	result.uv = uv;

	return result;
}

PSOutput PSMain(PSInput input) // : SV_TARGET
{
	PSOutput output;
	//const float2 screenPosition = (input.clipPos.xy / input.clipPos.w);
	//const float4 mainColor = g_texture.Sample(g_sampler, input.uv);
	//const float4 worldNormal = normalTexture.Sample(g_sampler, screenPosition);// g_texture.Sample(g_sampler, input.uv);
	//const float4 worldPos = positionTexture.Sample(g_sampler, screenPosition);// g_texture.Sample(g_sampler, input.uv);
	//float4 lightDir = normalize(lightPos - worldPos);
	//float diff = max(dot(worldNormal, lightDir), 0.0);
	//float ambient = 0.2f;

	output.Color = float4(input.uv, 0, 1); // mainColor* (ambient + diff);

	return output;
}
