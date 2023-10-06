#ifndef RAYTRACING_INCLUDE
#define RAYTRACING_INCLUDE

float2 SampleSphericalMap(float3 v)
{
    const float2 invAtan = float2(0.1591, 0.3183);
    float2 uv = float2(atan2(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    uv *= float2(-1, 1);

	//float r = length(v);
	//float theta = acos(-v.y);
	//float phi = atan2(v.x, -v.z);
	//float2 uv = float2(
	//	0.5 + phi / 2.0/ PI,
	//	theta / PI
	//	);
    return uv;
}

#endif
