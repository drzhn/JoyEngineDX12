#ifndef GRAPHICS_UTILS_H
#define GRAPHICS_UTILS_H

#include <cstdint>
#include <d3d12.h>

#include "CommonEngineStructs.h"
#include "ResourceManager/Pipelines/ComputePipeline.h"
#include "ResourceManager/Pipelines/GraphicsPipeline.h"
#include "ResourceManager/Pipelines/RaytracingPipeline.h"

#include "WinPixEventRuntime/pix3.h"
#define GFX_DEBUG_COLOR_RED PIX_COLOR(255,0,0)

namespace JoyEngine
{
	class ResourceView;

	class GraphicsUtils
	{
	public:
		static void AttachView(ID3D12GraphicsCommandList* commandList, const GraphicsPipeline* pipeline, const char* paramName, const ResourceView* view);
		static void AttachView(ID3D12GraphicsCommandList* commandList, const GraphicsPipeline* pipeline, uint32_t rootParamIndex, const ResourceView* view);

		static void AttachView(ID3D12GraphicsCommandList* commandList, const ComputePipeline* pipeline, const char* paramName, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle);
		static void AttachView(ID3D12GraphicsCommandList* commandList, const ComputePipeline* pipeline, const char* paramName, const ResourceView* view);
		static void AttachView(ID3D12GraphicsCommandList* commandList, const ComputePipeline* pipeline, uint32_t rootParamIndex, const ResourceView* view);

		static void AttachView(ID3D12GraphicsCommandList* commandList, const RaytracingPipeline* pipeline, const char* paramName, const ResourceView* view);
		static void AttachView(const RaytracingPipeline* pipeline, ShaderTableType shaderTableType, const char* paramName, const ResourceView* view);

		static void Barrier(ID3D12GraphicsCommandList* commandList, ID3D12Resource* pResource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter);
		static void UAVBarrier(ID3D12GraphicsCommandList* commandList, ID3D12Resource* pResource);
		static void SetViewportAndScissor(ID3D12GraphicsCommandList* commandList, uint32_t width, uint32_t height);

		static void ProcessEngineBindings(
			ID3D12GraphicsCommandList* commandList,
			const GraphicsPipeline* pipeline,
			uint32_t frameIndex,
			const uint32_t* modelIndex,
			const ViewProjectionMatrixData* viewProjectionMatrix);

		static void ProcessEngineBindings(
			ID3D12GraphicsCommandList* commandList,
			const ComputePipeline* pipeline,
			uint32_t frameIndex,
			const uint32_t* modelIndex,
			const ViewProjectionMatrixData* viewProjectionMatrix);

		static void BeginDebugEvent(ID3D12GraphicsCommandList* commandList, UINT64 color, char const* formatString, ...);
		static void EndDebugEvent(ID3D12GraphicsCommandList* commandList);
	};
}
#endif // GRAPHICS_UTILS_H
