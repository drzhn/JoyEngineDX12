struct PSInput
{
	float4 position : SV_POSITION;
	float4 worldPos: COLOR1;
	float4 worldNormal: COLOR2;
};

struct PSOutput
{
	float4 Position: SV_Target0;
	float4 Normal: SV_Target1;
};

struct MVP
{
	float4x4 model;
	float4x4 view;
	float4x4 projection;
};

ConstantBuffer<MVP> mvp : register(b0);

PSInput VSMain(float3 position : POSITION, float3 color : COLOR, float3 normal: NORMAL, float2 uv : TEXCOORD)
{
	PSInput result;
	float4x4 resMatrix = mul(mvp.projection, mul(mvp.view, mvp.model));
	result.position = mul(resMatrix, float4(position,1));
	result.worldPos = mul(mvp.model, float4(position,1));
	result.worldNormal = normalize(mul(mvp.model, float4(normal,0)));

	return result;
}

PSOutput PSMain(PSInput input)// : SV_TARGET
{
	PSOutput output;
	output.Position = input.worldPos;
	output.Normal = float4(input.worldNormal.rgb,1);
	return output;
}
