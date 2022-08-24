#ifndef GRAPHICS_UTILS_H
#define GRAPHICS_UTILS_H

#include <cstdint>
#include <d3d12.h>

namespace JoyEngine
{
	class ResourceView;

	class GraphicsUtils
	{
	public:
		static void AttachViewToGraphics(ID3D12GraphicsCommandList* commandList, uint32_t rootParameterIndex, const ResourceView* view);
		static void AttachViewToCompute(ID3D12GraphicsCommandList* commandList, uint32_t rootParameterIndex, const ResourceView* view);
		static void Barrier(ID3D12GraphicsCommandList* commandList, ID3D12Resource* pResource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter);
		static void SetViewportAndScissor(ID3D12GraphicsCommandList* commandList, uint32_t width, uint32_t height);
	};
}
#endif // GRAPHICS_UTILS_H
