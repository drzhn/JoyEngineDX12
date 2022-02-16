RWStructuredBuffer<float3> particles;

cbuffer CSData : register(b0)
{
float time;
}

[numthreads( 8, 8, 8 )]
void CSMain(uint GI : SV_GroupIndex, uint3 DTid : SV_DispatchThreadID)
{
	particles[DTid.x * 128 * 127 + DTid.y * 127 + DTid.z] = DTid / 128.0f;
}
