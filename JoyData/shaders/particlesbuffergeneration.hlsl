RWStructuredBuffer<float3> particles;

cbuffer CSData : register(b0)
{
float time;
}

static const float PI = 3.14159265f;
static const float ToRad = PI / 180;

[numthreads( 8, 8, 8 )]
void CSMain(uint GI : SV_GroupIndex, uint3 DTid : SV_DispatchThreadID)
{
	float3 coord = DTid / 127.0f;
	coord -= float3(0.5, 0.5, 0.5);
	float3 coordNorm = normalize(coord);
	coord = lerp(coord, coordNorm, sin(time * 2) - 2);

	particles[DTid.x * 128 * 128 + DTid.y * 128 + DTid.z] = coord;
}
