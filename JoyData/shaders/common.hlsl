struct EngineData
{
	float4 perspectiveValues;
	float3 cameraWorldPos;
	float time;
	float4x4 cameraInvProj;
};

struct MVP
{
	float4x4 model;
	float4x4 view;
	float4x4 projection;
};

struct HDRDownScaleConstants
{
	// Resolution of the down scaled target: x - width, y - height
	uint2 Res;
	// Total pixel in the downscaled image
	uint Domain;
	// Number of groups dispached on the first pass
	uint GroupSize;
	float Adaptation; // Adaptation factor
	float fBloomThreshold; // Bloom threshold percentage
};

struct DirectionLightData
{
	float3 direction;
	float intensity;
	float ambient;
};

struct LightData
{
	float intensity;
	float radius;
	float height;
	float angle;

	float4x4 view[6];
	float4x4 projection;
};

struct CubemapConvolutionConstants
{
	float4x4 model;
	float4x4 view[6];
	float4x4 projection;
};

struct SSAOData
{
	float4 offsetVectors[14];
	float2 invRenderTargetSize;
	uint isHorizontal;
	uint _dummy;
	float4 blurWeights[3];
};