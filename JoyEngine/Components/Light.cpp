#include "Light.h"


#include "MeshRenderer.h"
#include "DataManager/DataManager.h"
#include "EngineMaterialProvider/EngineMaterialProvider.h"
#include "ResourceManager/Texture.h"
#include "RenderManager/RenderManager.h"
#include "SceneManager/GameObject.h"
#include "SceneManager/Transform.h"
#include "Utils/GraphicsUtils.h"


namespace JoyEngine
{
	#define DIRECTIONAL_SHADOWMAP_SIZE 2048

	//Light::Light(LightType lightType, float intensity, float radius, float height, float angle, float ambient):
	//	m_lightType(lightType),
	//	m_intensity(intensity),
	//	m_radius(radius),
	//	m_height(height),
	//	m_angle(angle),
	//	m_ambient(ambient)
	//{
	//	if (lightType == Spot)
	//	{
	//		m_shadowmap = std::make_unique<DepthTexture>(
	//			512,
	//			512,
	//			DXGI_FORMAT_R32_TYPELESS,
	//			D3D12_RESOURCE_STATE_GENERIC_READ,
	//			D3D12_HEAP_TYPE_DEFAULT);

	//		m_cameraUnit = CameraUnit(
	//			1,
	//			512,
	//			512,
	//			m_angle,
	//			0.1f,
	//			1000
	//		);
	//	}

	//	if (lightType == Point)
	//	{
	//		m_shadowmap = std::make_unique<DepthTexture>(
	//			512,
	//			512,
	//			DXGI_FORMAT_R32_TYPELESS,
	//			D3D12_RESOURCE_STATE_GENERIC_READ,
	//			D3D12_HEAP_TYPE_DEFAULT,
	//			6);

	//		m_cameraUnit = CameraUnit(
	//			1,
	//			512,
	//			512,
	//			90,
	//			0.1f,
	//			1000
	//		);
	//	}


	//	uint32_t bufferSize = ((sizeof(LightData) - 1) / 256 + 1) * 256; // Device requirement. TODO check this 
	//	m_lightDataBuffer = std::make_unique<Buffer>(bufferSize, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_HEAP_TYPE_UPLOAD);
	//	m_lightDataBufferView = std::make_unique<ResourceView>(
	//		D3D12_CONSTANT_BUFFER_VIEW_DESC{
	//			m_lightDataBuffer->GetBufferResource()->GetGPUVirtualAddress(),
	//			bufferSize
	//		}
	//	);
	//}

	//void Light::Enable()
	//{
	//	if (m_lightType == Direction)
	//	{
	//		RenderManager::Get()->RegisterDirectionLight(this);
	//	}
	//	else
	//	{
	//		RenderManager::Get()->RegisterLight(this);
	//	}
	//}

	//void Light::Disable()
	//{
	//	if (m_lightType == Direction)
	//	{
	//		RenderManager::Get()->UnregisterDirectionLight(this);
	//	}
	//	else
	//	{
	//		RenderManager::Get()->UnregisterLight(this);
	//	}
	//}

	//void Light::Update()
	//{
	//	const auto ptr = m_lightDataBuffer->GetMappedPtr();
	//	const auto data = static_cast<LightData*>(ptr->GetMappedPtr());

	//	// shut up :)
	//	data->intensity = m_intensity;
	//	data->radius = m_radius;
	//	data->height = m_height;
	//	data->angle = m_angle;

	//	if (m_lightType == Spot)
	//	{
	//		data->view[0] = GetViewMatrix();
	//		data->proj = GetProjMatrix();
	//	}

	//	if (m_lightType == Point)
	//	{
	//		data->view[0] = GetCubeViewMatrix(0);
	//		data->view[1] = GetCubeViewMatrix(1);
	//		data->view[2] = GetCubeViewMatrix(2);
	//		data->view[3] = GetCubeViewMatrix(3);
	//		data->view[4] = GetCubeViewMatrix(4);
	//		data->view[5] = GetCubeViewMatrix(5);
	//		data->proj = GetProjMatrix();
	//	}
	//}

	//glm::mat4 Light::GetViewMatrix() const
	//{
	//	return m_cameraUnit.GetViewMatrix(m_transform->GetPosition(), m_transform->GetRotation());
	//}

	//glm::mat4 Light::GetCubeViewMatrix(uint32_t index) const
	//{
	//	const glm::vec3 eye = m_transform->GetPosition();
	//	return glm::lookAtLH(eye, eye + shadowTransformsForward[index], shadowTransformsUp[index]);
	//}

	//glm::mat4x4 Light::GetProjMatrix() const
	//{
	//	return m_cameraUnit.GetProjMatrix();
	//}

	DirectionalLight::DirectionalLight(GameObject& go, IRenderManager* renderManager, float intensity, float ambient)
		: Light(go, renderManager),
		  m_cameraUnit(1, 50, 0.1f, 1000.0f)
	{
		m_lightData.ambient = ambient;
		m_lightData.intensity = intensity;
		m_lightData.bias = 0.001f;
		m_lightData.shadowmapSize = DIRECTIONAL_SHADOWMAP_SIZE;

		m_shadowmap = std::make_unique<DepthTexture>(
			DIRECTIONAL_SHADOWMAP_SIZE,
			DIRECTIONAL_SHADOWMAP_SIZE,
			DXGI_FORMAT_R32_TYPELESS,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			D3D12_HEAP_TYPE_DEFAULT);
	}


	void DirectionalLight::Enable()
	{
		m_renderManager->RegisterDirectionLight(this);
		m_currentAngle = 30;
	}

	void DirectionalLight::Disable()
	{
		m_renderManager->UnregisterDirectionLight(this);
	}

	void DirectionalLight::Update()
	{
		// TODO this should be controlled by behaviour class
		m_gameObject.GetTransform()->SetRotation(glm::vec3(m_currentAngle, 180, 0));
		m_gameObject.GetTransform()->SetPosition(glm::vec3(
			0,
			70 * glm::cos(glm::radians(90 - m_currentAngle)),
			70 * glm::sin(glm::radians(90 - m_currentAngle))));
		m_lightData.direction = m_gameObject.GetTransform()->GetForward();
		m_lightData.proj = m_cameraUnit.GetProjMatrix();
		m_lightData.view = m_cameraUnit.GetViewMatrix(m_gameObject.GetTransform()->GetPosition(), m_gameObject.GetTransform()->GetRotation());
	}

	void DirectionalLight::RenderShadows(
		ID3D12GraphicsCommandList* commandList,
		uint32_t frameIndex,
		SharedMaterial* gBufferSharedMaterial)
	{
		m_lightDataBuffer->SetData(&m_lightData, frameIndex);

		GraphicsUtils::Barrier(commandList, m_shadowmap->GetImageResource().Get(), D3D12_RESOURCE_STATE_GENERIC_READ,
		                       D3D12_RESOURCE_STATE_DEPTH_WRITE);

		GraphicsUtils::SetViewportAndScissor(commandList, m_shadowmap->GetWidth(), m_shadowmap->GetHeight());

		const auto shadowMapHandle = m_shadowmap->GetDSV()->GetCPUHandle();

		commandList->OMSetRenderTargets(
			0,
			nullptr,
			FALSE, &shadowMapHandle);

		commandList->ClearDepthStencilView(shadowMapHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

		const auto sm = EngineMaterialProvider::Get()->GetShadowProcessingSharedMaterial();
		commandList->SetPipelineState(sm->GetGraphicsPipeline()->GetPipelineObject().Get());
		commandList->SetGraphicsRootSignature(sm->GetGraphicsPipeline()->GetRootSignature().Get());

		const ViewProjectionMatrixData viewProjectionMatrixData = {
			.view = m_lightData.view,
			.proj = m_lightData.proj,
		};

		for (const auto& mr : gBufferSharedMaterial->GetMeshRenderers())
		{
			commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			commandList->IASetVertexBuffers(0, 1, mr->GetMesh()->GetVertexBufferView());
			commandList->IASetIndexBuffer(mr->GetMesh()->GetIndexBufferView());

			GraphicsUtils::ProcessEngineBindings(
				commandList,
				frameIndex,
				sm->GetGraphicsPipeline()->GetEngineBindings(),
				mr->GetGameObject().GetTransformIndexPtr(),
				&viewProjectionMatrixData);

			commandList->DrawIndexedInstanced(
				mr->GetMesh()->GetIndexCount(),
				1,
				0, 0, 0);
		}

		GraphicsUtils::Barrier(commandList, m_shadowmap->GetImageResource().Get(), D3D12_RESOURCE_STATE_DEPTH_WRITE,
		                       D3D12_RESOURCE_STATE_GENERIC_READ);
	}
}
