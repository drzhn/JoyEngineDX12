#include "CommonEngineStructs.h"

ConstantBuffer<MVP> mvp : register(b0);


struct PSInput
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

PSInput VSMain(uint id : SV_VertexID)
{
	PSInput input;

	// 0 -- 0 0 0
	// 1 -- 1 0 0 
	// 2 -- 0 0 0
	// 3 -- 0 1 0
	// 4 -- 0 0 0
	// 5 -- 0 0 1

	input.position = float4(id, id, 0, 1);

	return input;
}

float4 PSMain(PSInput input) : SV_Target
{
	return float4(1,1,1,1);
}
