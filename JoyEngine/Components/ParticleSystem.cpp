#include "ParticleSystem.h"

#include "RenderManager/JoyTypes.h"
#include "RenderManager/RenderManager.h"
#include "ResourceManager/Buffer.h"
#include "ResourceManager/SharedMaterial.h"
#include "Utils/DummyMaterialProvider.h"


namespace JoyEngine
{
	void ParticleSystem::Enable()
	{
		JoyContext::Render->RegisterParticleSystem(this);

		m_buffer = std::make_unique<Buffer>(
			m_size * m_size * m_size * sizeof(glm::vec3), 
			D3D12_RESOURCE_STATE_GENERIC_READ, 
			D3D12_HEAP_TYPE_DEFAULT,
			D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		D3D12_UNORDERED_ACCESS_VIEW_DESC desc;
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		desc.Buffer = {
			0,
			m_size * m_size * m_size,
			sizeof(glm::vec3),
			0
		};
		m_bufferView = std::make_unique<ResourceView>(desc, m_buffer->GetBuffer().Get());

		{
			const GUID shaderGuid = GUID::StringToGuid("a36fff56-b183-418a-9bd1-31cffd247e37"); // shaders/particles.hlsl
			const GUID sharedMaterialGuid = GUID::Random();

			RootParams rp;
			rp.CreateConstants(sizeof(MVP) / 4, 0);
			rp.CreateDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0);

			//rp.CreateDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, D3D12_SHADER_VISIBILITY_PIXEL);
			//rp.CreateDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 0, D3D12_SHADER_VISIBILITY_PIXEL);
			//rp.CreateConstants(sizeof(MVP) / 4, 0);
			//rp.CreateDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, D3D12_SHADER_VISIBILITY_PIXEL);

			m_sharedMaterial = JoyContext::Resource->LoadResource<SharedMaterial, SharedMaterialArgs>(
				sharedMaterialGuid,
				{
					shaderGuid,
					JoyShaderTypeVertex | JoyShaderTypePixel | JoyShaderTypeGeometry,
					false,
					true,
					true,
					D3D12_CULL_MODE_BACK,
					D3D12_COMPARISON_FUNC_LESS_EQUAL,
					CD3DX12_BLEND_DESC(D3D12_DEFAULT),
					rp.params,
					{
						DXGI_FORMAT_R8G8B8A8_UNORM
					},
					DXGI_FORMAT_D32_FLOAT,
					D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT
				});
		}

		// Buffer generation
		{
			const GUID bufferGenerationShaderGuid = GUID::StringToGuid("38d4f011-405a-4602-8f8e-79b4888d26b6"); //shaders/particlesbuffergeneration.hlsl
			const GUID bufferGenerationPipelineGuid = GUID::Random();

			RootParams rp;
			rp.CreateDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0);
			rp.CreateConstants(sizeof(float), 0);

			m_computePipeline = JoyContext::Resource->LoadResource<ComputePipeline, ComputePipelineArgs>(
				bufferGenerationPipelineGuid,
				{
					bufferGenerationShaderGuid,
					rp.params
				});
		}
	}

	void ParticleSystem::Disable()
	{
		JoyContext::Render->UnregisterParticleSystem(this);
	}

	void ParticleSystem::Update()
	{
	}
}
