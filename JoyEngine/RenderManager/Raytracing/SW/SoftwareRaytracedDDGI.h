#ifndef SOFTWARE_RAYTRACED_DDGI_H
#define SOFTWARE_RAYTRACED_DDGI_H

#include <d3d12.h>

#include <memory>
#include "CommonEngineStructs.h"
#include "BufferSorter.h"
#include "BVHConstructor.h"
#include "RenderManager/GBuffer.h"
#include "RenderManager/Raytracing/RaytracedDDGIDataContainer.h"
#include "ResourceManager/SharedMaterial.h"

namespace JoyEngine
{
	class IRenderManager;

	class SoftwareRaytracedDDGI
	{
	public:
		SoftwareRaytracedDDGI() = delete;
		explicit SoftwareRaytracedDDGI(
			const RaytracedDDGIDataContainer& dataContainer,
			DXGI_FORMAT mainColorFormat,
			DXGI_FORMAT swapchainFormat,
			uint32_t width,
			uint32_t height);

		void UploadSceneData();
		void PrepareBVH() const;
		void ProcessRaytracing(ID3D12GraphicsCommandList* commandList, const uint32_t frameIndex, const ViewProjectionMatrixData* data);
		void GenerateProbeIrradiance(ID3D12GraphicsCommandList* commandList, uint32_t frameIndex) const;
		void DebugDrawRaytracedImage(ID3D12GraphicsCommandList* commandList) const;
		void DebugDrawAABBGizmo(ID3D12GraphicsCommandList* commandList, const ViewProjectionMatrixData* viewProjectionMatrixData) const;
		void DebugDrawProbes(ID3D12GraphicsCommandList* commandList, uint32_t frameIndex, const ViewProjectionMatrixData* viewProjectionMatrixData) const;

		[[nodiscard]] UAVGbuffer* GetGBuffer() const { return m_gbuffer.get(); }
		[[nodiscard]] RenderTexture* GetShadedRenderTexture() const { return m_shadedRenderTexture.get(); }
		[[nodiscard]] UAVTexture* GetProbeIrradianceTexture() const { return m_probeIrradianceTexture.get(); }
		[[nodiscard]] UAVTexture* GetProbeDepthTexture() const { return m_probeDepthTexture.get(); }
		[[nodiscard]] uint32_t GetRaytracedTextureWidth() const noexcept { return m_raytracedTextureWidth; }
		[[nodiscard]] uint32_t GetRaytracedTextureHeight() const noexcept { return m_raytracedTextureHeight; }
		[[nodiscard]] RaytracedProbesData* GetRaytracedProbesDataPtr() noexcept;

	private:
		const RaytracedDDGIDataContainer& m_dataContainer;

		uint32_t m_raytracedTextureWidth;
		uint32_t m_raytracedTextureHeight;

		uint32_t m_trianglesLength = 0;

		std::unique_ptr<UAVGbuffer> m_gbuffer;
		std::unique_ptr<RenderTexture> m_shadedRenderTexture;
		std::unique_ptr<UAVTexture> m_probeIrradianceTexture; // octohedral irradince per-probe storage
		std::unique_ptr<UAVTexture> m_probeDepthTexture; // octohedral depth per-probe storage


		std::unique_ptr<DataBuffer<uint32_t>> m_keysBuffer;
		std::unique_ptr<DataBuffer<uint32_t>> m_triangleIndexBuffer;
		std::unique_ptr<DataBuffer<AABB>> m_triangleAABBBuffer;

		std::unique_ptr<DataBuffer<AABB>> m_bvhDataBuffer;
		std::unique_ptr<DataBuffer<LeafNode>> m_bvhLeafNodesBuffer;
		std::unique_ptr<DataBuffer<InternalNode>> m_bvhInternalNodesBuffer;

		ConstantCpuBuffer<BVHConstructorData> m_bvhConstructionData;

		std::unique_ptr<BufferSorter> m_bufferSorter;
		std::unique_ptr<BVHConstructor> m_bvhConstructor;

		std::unique_ptr<ComputePipeline> m_raytracingPipeline;
		std::unique_ptr<ComputePipeline> m_probeIrradiancePipeline;

		std::unique_ptr<GraphicsPipeline> m_debugGizmoAABBDrawerGraphicsPipeline;
	};
}
#endif // SOFTWARE_RAYTRACED_DDGI_H
