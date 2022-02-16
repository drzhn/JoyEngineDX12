struct PSInput
{
	float4 position : SV_POSITION;
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
RWStructuredBuffer<float3> particles: register(u0);

PSInput VSMain(uint VertexID : SV_VertexID)
{
	PSInput result;
	float4x4 resMatrix = mul(mvp.projection, mul(mvp.view, mvp.model));
	result.position = mul(resMatrix, float4(particles[VertexID], 1));

	return result;
}

[maxvertexcount(1)] 
void GSMain(point PSInput input[1], inout PointStream<PSInput> stream)
{
	PSInput pointOut = input[0]; 
	stream.Append(pointOut); 
	stream.RestartStrip(); 
}

PSOutput PSMain(PSInput input) // : SV_TARGET
{
	PSOutput output;
	output.Color = float4(1, 1, 1, 1);

	return output;
}
