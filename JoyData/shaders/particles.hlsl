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

ConstantBuffer<MVP> mvp : register(b0);
Texture2D lightAttachment : register(t0);

inline float4 ComputeNonStereoScreenPos(float4 pos)
{
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

[maxvertexcount(3)]
void GSMain(triangle PSInput input[3], inout TriangleStream<PSInput> stream)
{
	for (int i = 0; i < 3; i++)
	{
		PSInput pointOut = input[i];
		stream.Append(pointOut);
	}
	stream.RestartStrip();
}

PSOutput PSMain(PSInput input) // : SV_TARGET
{
	PSOutput output;
	const float4 light = lightAttachment.Load(float3(input.position.xy, 0)); // normalTexture.Sample(g_sampler, screenPosition);// g_texture.Sample(g_sampler, input.uv);

	output.Color = light;

	return output;
}
