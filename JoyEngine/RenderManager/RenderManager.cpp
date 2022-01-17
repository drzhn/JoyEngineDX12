#include "RenderManager.h"

#include <memory>

#include "JoyContext.h"
#include "Utils/Assert.h"

//#include "MemoryManager/MemoryManager.h"
//#include "ResourceManager/ResourceManager.h"
#include "JoyTypes.h"
#include "Components/MeshRenderer.h"
#include "DescriptorManager/DescriptorManager.h"
#include "GraphicsManager/GraphicsManager.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

namespace JoyEngine
{
	RenderManager::RenderManager()
	{
	}

	void RenderManager::Init()
	{
		RECT rect;

		if (GetWindowRect(JoyContext::Graphics->GetHWND(), &rect))
		{
			m_width = rect.right - rect.left;
			m_height = rect.bottom - rect.top;
		}

		ASSERT(m_width != 0 && m_height != 0);

		m_viewport = {
			0.0f,
			0.0f,
			static_cast<float>(m_width),
			static_cast<float>(m_height),
			D3D12_MIN_DEPTH,
			D3D12_MAX_DEPTH
		};

		m_scissorRect = {
			0,
			0,
			static_cast<LONG>(m_width),
			static_cast<LONG>(m_height)
		};

		m_queue = std::make_unique<CommandQueue>(D3D12_COMMAND_LIST_TYPE_DIRECT, JoyContext::Graphics->GetDevice(),
		                                         FrameCount);

		// Describe and create the swap chain.
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.BufferCount = FrameCount;
		swapChainDesc.Width = m_width;
		swapChainDesc.Height = m_height;
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.SampleDesc.Count = 1;

		ComPtr<IDXGISwapChain1> swapChain;
		ASSERT_SUCC(JoyContext::Graphics->GetFactory()->CreateSwapChainForHwnd(
			m_queue->GetQueue(), // Swap chain needs the queue so that it can force a flush on it.
			JoyContext::Graphics->GetHWND(),
			&swapChainDesc,
			nullptr,
			nullptr,
			&swapChain
		));

		// This sample does not support fullscreen transitions.
		ASSERT_SUCC(
			JoyContext::Graphics->GetFactory()->MakeWindowAssociation(JoyContext::Graphics->GetHWND(),
				DXGI_MWA_NO_ALT_ENTER));
		ASSERT_SUCC(swapChain.As(&m_swapChain));

		m_currentFrameIndex = m_swapChain->GetCurrentBackBufferIndex();

		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.NumDescriptors = FrameCount;
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

		ASSERT_SUCC(JoyContext::Graphics->GetDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_RTVDescriptorHeap)));
		auto rtvDescriptorSize = JoyContext::Graphics->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

		// Create a RTV and a command allocator for each frame.
		for (UINT n = 0; n < FrameCount; n++)
		{
			ASSERT_SUCC(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n].resource)));
			m_renderTargets[n].handle = rtvHandle;
			JoyContext::Graphics->GetDevice()->CreateRenderTargetView(
				m_renderTargets[n].resource.Get(),
				nullptr,
				m_renderTargets[n].handle);
			rtvHandle.ptr += rtvDescriptorSize;
		}
	}


	void RenderManager::Stop()
	{
		m_queue = nullptr;
	}

	RenderManager::~RenderManager()
	{
		//m_depthAttachment = nullptr;
		//m_normalAttachment = nullptr;
		//m_positionAttachment = nullptr;

		//for (size_t i = 0; i < m_swapChainFramebuffers.size(); i++)
		//{
		//	vkDestroyFramebuffer(JoyContext::Graphics->GetDevice(), m_swapChainFramebuffers[i],
		//	                     JoyContext::Graphics->GetAllocationCallbacks());
		//}

		//vkFreeCommandBuffers(JoyContext::Graphics->GetDevice(),
		//                     JoyContext::Graphics->GetCommandPool(),
		//                     static_cast<uint32_t>(commandBuffers.size()),
		//                     commandBuffers.data());

		//m_renderPass = nullptr;

		//m_swapchain = nullptr;

		//for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		//{
		//	vkDestroySemaphore(JoyContext::Graphics->GetDevice(), m_renderFinishedSemaphores[i],
		//	                   JoyContext::Graphics->GetAllocationCallbacks());
		//	vkDestroySemaphore(JoyContext::Graphics->GetDevice(), m_imageAvailableSemaphores[i],
		//	                   JoyContext::Graphics->GetAllocationCallbacks());
		//	vkDestroyFence(JoyContext::Graphics->GetDevice(), m_inFlightFences[i],
		//	               JoyContext::Graphics->GetAllocationCallbacks());
		//}
	}

	//void RenderManager::CreateRenderPass()
	//{
	//	const VkFormat depthFormat = findDepthFormat(JoyContext::Graphics->GetPhysicalDevice());
	//	const VkFormat colorFormat = m_swapchain->GetSwapChainImageFormat();
	//	const VkFormat positionFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
	//	const VkFormat normalFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
	//	m_depthAttachment = std::make_unique<Texture>(
	//		m_swapchain->GetWidth(),
	//		m_swapchain->GetHeight(),
	//		depthFormat,
	//		VK_IMAGE_TILING_OPTIMAL,
	//		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
	//		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
	//		VK_IMAGE_ASPECT_DEPTH_BIT
	//	);
	//	m_positionAttachment = std::make_unique<Texture>(
	//		m_swapchain->GetWidth(),
	//		m_swapchain->GetHeight(),
	//		positionFormat,
	//		VK_IMAGE_TILING_OPTIMAL,
	//		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
	//		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
	//		VK_IMAGE_ASPECT_COLOR_BIT
	//	);
	//	m_normalAttachment = std::make_unique<Texture>(
	//		m_swapchain->GetWidth(),
	//		m_swapchain->GetHeight(),
	//		normalFormat,
	//		VK_IMAGE_TILING_OPTIMAL,
	//		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
	//		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
	//		VK_IMAGE_ASPECT_COLOR_BIT
	//	);
	//	const VkAttachmentDescription colorAttachmentDescription = {
	//		0,
	//		colorFormat,
	//		VK_SAMPLE_COUNT_1_BIT, // TODO later
	//		VK_ATTACHMENT_LOAD_OP_CLEAR,
	//		VK_ATTACHMENT_STORE_OP_STORE,
	//		VK_ATTACHMENT_LOAD_OP_DONT_CARE,
	//		VK_ATTACHMENT_STORE_OP_DONT_CARE,
	//		VK_IMAGE_LAYOUT_UNDEFINED,
	//		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
	//	};
	//	const VkAttachmentDescription positionGBufferAttachmentDescription = {
	//		0,
	//		positionFormat,
	//		VK_SAMPLE_COUNT_1_BIT, // TODO later
	//		VK_ATTACHMENT_LOAD_OP_CLEAR,
	//		VK_ATTACHMENT_STORE_OP_DONT_CARE,
	//		VK_ATTACHMENT_LOAD_OP_DONT_CARE,
	//		VK_ATTACHMENT_STORE_OP_DONT_CARE,
	//		VK_IMAGE_LAYOUT_UNDEFINED,
	//		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	//	};
	//	const VkAttachmentDescription normalGBufferAttachmentDescription = {
	//		0,
	//		normalFormat,
	//		VK_SAMPLE_COUNT_1_BIT, // TODO later
	//		VK_ATTACHMENT_LOAD_OP_CLEAR,
	//		VK_ATTACHMENT_STORE_OP_DONT_CARE,
	//		VK_ATTACHMENT_LOAD_OP_DONT_CARE,
	//		VK_ATTACHMENT_STORE_OP_DONT_CARE,
	//		VK_IMAGE_LAYOUT_UNDEFINED,
	//		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	//	};
	//	const VkAttachmentDescription depthAttachmentDescription = {
	//		0,
	//		depthFormat,
	//		VK_SAMPLE_COUNT_1_BIT, // TODO later
	//		VK_ATTACHMENT_LOAD_OP_CLEAR,
	//		VK_ATTACHMENT_STORE_OP_DONT_CARE,
	//		VK_ATTACHMENT_LOAD_OP_DONT_CARE,
	//		VK_ATTACHMENT_STORE_OP_DONT_CARE,
	//		VK_IMAGE_LAYOUT_UNDEFINED,
	//		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
	//	};
	//	VkAttachmentDescription attachments[] = {
	//		colorAttachmentDescription,
	//		positionGBufferAttachmentDescription,
	//		normalGBufferAttachmentDescription,
	//		depthAttachmentDescription
	//	};
	//	constexpr VkAttachmentReference colorAttachmentRef = {
	//		0,
	//		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	//	};
	//	constexpr VkAttachmentReference positionGBufferAttachmentRef = {
	//		1,
	//		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	//	};
	//	constexpr VkAttachmentReference normalGBufferAttachmentRef = {
	//		2,
	//		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	//	};
	//	constexpr VkAttachmentReference positionInputAttachmentRef = {
	//		1,
	//		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
	//	};
	//	constexpr VkAttachmentReference normalInputAttachmentRef = {
	//		2,
	//		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
	//	};
	//	constexpr VkAttachmentReference depthAttachmentRef = {
	//		3,
	//		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
	//	};
	//	constexpr VkAttachmentReference gbufferColorAttachmentRefs[] = {
	//		positionGBufferAttachmentRef,
	//		normalGBufferAttachmentRef
	//	};
	//	constexpr VkAttachmentReference inputAttachmentRefs[] = {
	//		positionInputAttachmentRef,
	//		normalInputAttachmentRef
	//	};
	//	const VkSubpassDescription subpasses[] = {
	//		{
	//			0,
	//			VK_PIPELINE_BIND_POINT_GRAPHICS,
	//			0,
	//			nullptr,
	//			2,
	//			gbufferColorAttachmentRefs,
	//			nullptr,
	//			&depthAttachmentRef,
	//			0,
	//			nullptr,
	//		},
	//		{
	//			0,
	//			VK_PIPELINE_BIND_POINT_GRAPHICS,
	//			2,
	//			inputAttachmentRefs,
	//			1,
	//			&colorAttachmentRef,
	//			nullptr,
	//			&depthAttachmentRef,
	//			0,
	//			nullptr,
	//		}
	//	};
	//	VkSubpassDependency dependencies[] =
	//	{
	//		{
	//			VK_SUBPASS_EXTERNAL,
	//			0,
	//			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
	//			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
	//			VK_ACCESS_MEMORY_READ_BIT,
	//			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
	//			VK_DEPENDENCY_BY_REGION_BIT
	//		},
	//		{
	//			0,
	//			1,
	//			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
	//			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
	//			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
	//			VK_ACCESS_SHADER_READ_BIT,
	//			VK_DEPENDENCY_BY_REGION_BIT
	//		},
	//		{
	//			1,
	//			VK_SUBPASS_EXTERNAL,
	//			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
	//			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
	//			VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
	//			VK_ACCESS_MEMORY_READ_BIT,
	//			VK_DEPENDENCY_BY_REGION_BIT
	//		}
	//	};
	//	m_renderPass = std::make_unique<RenderPass>(
	//		4,
	//		attachments,
	//		2,
	//		subpasses,
	//		3,
	//		dependencies);
	//	m_gBufferWriteSharedMaterial = GUID::StringToGuid("869fa59b-d775-41fb-9650-d3f9e8f72269");
	//	m_commonDescriptorSetProvider = std::make_unique<CommonDescriptorSetProvider>();
	//}
	//void RenderManager::CreateFramebuffers()
	//{
	//	m_swapChainFramebuffers.resize(m_swapchain->GetSwapchainImageCount());
	//	for (size_t i = 0; i < m_swapchain->GetSwapchainImageCount(); i++)
	//	{
	//		VkImageView attachments[] = {
	//			m_swapchain->GetSwapChainImageViews()[i],
	//			m_positionAttachment->GetImageView(),
	//			m_normalAttachment->GetImageView(),
	//			m_depthAttachment->GetImageView()
	//		};
	//		VkFramebufferCreateInfo framebufferInfo{
	//			VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
	//			nullptr,
	//			0,
	//			m_renderPass->GetRenderPass(),
	//			4,
	//			attachments,
	//			m_swapchain->GetWidth(),
	//			m_swapchain->GetHeight(),
	//			1
	//		};
	//		const VkResult res = vkCreateFramebuffer(
	//			JoyContext::Graphics->GetDevice(),
	//			&framebufferInfo,
	//			JoyContext::Graphics->GetAllocationCallbacks(),
	//			&m_swapChainFramebuffers[i]);
	//		ASSERT_DESC(res == VK_SUCCESS, ParseVkResult(res));
	//	}
	//}

	void RenderManager::RegisterSharedMaterial(SharedMaterial* meshRenderer)
	{
		m_sharedMaterials.insert(meshRenderer);
	}

	void RenderManager::UnregisterSharedMaterial(SharedMaterial* meshRenderer)
	{
		if (m_sharedMaterials.find(meshRenderer) == m_sharedMaterials.end())
		{
			ASSERT(false);
		}
		m_sharedMaterials.erase(meshRenderer);
	}

	void RenderManager::RegisterCamera(Camera* camera)
	{
		m_currentCamera = camera;
		//m_commonDescriptorSetProvider->SetCamera(camera);
	}

	void RenderManager::UnregisterCamera(Camera* camera)
	{
		ASSERT(m_currentCamera == camera);
		m_currentCamera = nullptr;
	}

	//void RenderManager::CreateCommandBuffers()
	//{
	//	commandBuffers.resize(m_swapChainFramebuffers.size());
	//	const VkCommandBufferAllocateInfo allocInfo = {
	//		VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
	//		0,
	//		JoyContext::Graphics->GetCommandPool(),
	//		VK_COMMAND_BUFFER_LEVEL_PRIMARY,
	//		(uint32_t)commandBuffers.size()
	//	};
	//	const VkResult res = vkAllocateCommandBuffers(
	//		JoyContext::Graphics->GetDevice(),
	//		&allocInfo,
	//		commandBuffers.data());
	//	ASSERT(res == VK_SUCCESS)
	//}

	//void RenderManager::WriteCommandBuffers(uint32_t imageIndex) const
	//{
	//	VkCommandBufferBeginInfo beginInfo{};
	//	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	//	if (vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS)
	//	{
	//		throw std::runtime_error("failed to begin recording command buffer!");
	//	}
	//	VkClearValue clearValues[4];
	//	clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
	//	clearValues[1].color = {0.0f, 0.0f, 0.0f, 1.0f};
	//	clearValues[2].color = {0.0f, 0.0f, 0.0f, 1.0f};
	//	clearValues[3].depthStencil = {1.0f, 0};
	//	VkRenderPassBeginInfo renderPassInfo = {
	//		VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
	//		nullptr,
	//		m_renderPass->GetRenderPass(),
	//		m_swapChainFramebuffers[imageIndex],
	//		{
	//			{0, 0},
	//			{m_swapchain->GetWidth(), m_swapchain->GetHeight()}
	//		},
	//		4,
	//		clearValues
	//	};
	//	vkCmdBeginRenderPass(commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	//	ASSERT(m_currentCamera != nullptr);
	//	glm::mat4 view = m_currentCamera->GetViewMatrix();
	//	glm::mat4 proj = m_currentCamera->GetProjMatrix();
	//	vkCmdBindPipeline(
	//		commandBuffers[imageIndex],
	//		VK_PIPELINE_BIND_POINT_GRAPHICS,
	//		m_gBufferWriteSharedMaterial->GetPipeline());
	//	for (auto const& sm : m_sharedMaterials)
	//	{
	//		for (const auto& mr : sm->GetMeshRenderers())
	//		{
	//			if (!mr->IsReady()) continue;

	//			VkBuffer vertexBuffers[] = {
	//				mr->GetMesh()->GetVertexBuffer()
	//			};
	//			VkDeviceSize offsets[] = {0};
	//			vkCmdBindVertexBuffers(
	//				commandBuffers[imageIndex],
	//				0,
	//				1,
	//				vertexBuffers,
	//				offsets);
	//			vkCmdBindIndexBuffer(
	//				commandBuffers[imageIndex],
	//				mr->GetMesh()->GetIndexBuffer(),
	//				0,
	//				VK_INDEX_TYPE_UINT32);
	//			MVP mvp{
	//				mr->GetTransform()->GetModelMatrix(),
	//				view,
	//				proj
	//			};
	//			vkCmdPushConstants(
	//				commandBuffers[imageIndex],
	//				m_gBufferWriteSharedMaterial->GetPipelineLayout(),
	//				VK_SHADER_STAGE_VERTEX_BIT,
	//				0,
	//				sizeof(MVP),
	//				&mvp);
	//			vkCmdDrawIndexed(
	//				commandBuffers[imageIndex],
	//				static_cast<uint32_t>(mr->GetMesh()->GetIndexSize()),
	//				1,
	//				0,
	//				0,
	//				0);
	//		}
	//	}
	//	vkCmdNextSubpass(commandBuffers[imageIndex], VK_SUBPASS_CONTENTS_INLINE);
	//	for (auto const& sm : m_sharedMaterials)
	//	{
	//		vkCmdBindPipeline(
	//			commandBuffers[imageIndex],
	//			VK_PIPELINE_BIND_POINT_GRAPHICS,
	//			sm->GetPipeline());
	//		for (const auto& def : sm->GetBindingDefines())
	//		{
	//			SharedBindingData* data = m_commonDescriptorSetProvider->GetBindingData(def);
	//			vkCmdBindDescriptorSets(
	//				commandBuffers[imageIndex],
	//				VK_PIPELINE_BIND_POINT_GRAPHICS,
	//				sm->GetPipelineLayout(),
	//				data->setIndex,
	//				1,
	//				&data->descriptorSets[imageIndex],
	//				0, nullptr);
	//		}
	//		for (const auto& mr : sm->GetMeshRenderers())
	//		{
	//			if (!mr->IsReady()) continue;
	//			VkBuffer vertexBuffers[] = {
	//				mr->GetMesh()->GetVertexBuffer()
	//			};
	//			VkDeviceSize offsets[] = {0};
	//			vkCmdBindVertexBuffers(
	//				commandBuffers[imageIndex],
	//				0,
	//				1,
	//				vertexBuffers,
	//				offsets);
	//			vkCmdBindIndexBuffer(
	//				commandBuffers[imageIndex],
	//				mr->GetMesh()->GetIndexBuffer(),
	//				0,
	//				VK_INDEX_TYPE_UINT32);
	//			std::vector<VkDescriptorSet> sets = mr->GetMaterial()->GetDescriptorSets();
	//			vkCmdBindDescriptorSets(
	//				commandBuffers[imageIndex],
	//				VK_PIPELINE_BIND_POINT_GRAPHICS,
	//				sm->GetPipelineLayout(),
	//				0,
	//				1,
	//				&sets[imageIndex],
	//				0, nullptr);
	//			MVP mvp{
	//				mr->GetTransform()->GetModelMatrix(),
	//				view,
	//				proj
	//			};
	//			vkCmdPushConstants(
	//				commandBuffers[imageIndex],
	//				sm->GetPipelineLayout(),
	//				VK_SHADER_STAGE_VERTEX_BIT,
	//				0,
	//				sizeof(MVP),
	//				&mvp);
	//			vkCmdDrawIndexed(
	//				commandBuffers[imageIndex],
	//				static_cast<uint32_t>(mr->GetMesh()->GetIndexSize()),
	//				1,
	//				0,
	//				0,
	//				0);
	//		}
	//	}
	//	vkCmdEndRenderPass(commandBuffers[imageIndex]);

	//	if (vkEndCommandBuffer(commandBuffers[imageIndex]) != VK_SUCCESS)
	//	{
	//		throw std::runtime_error("failed to record command buffer!");
	//	}
	//}

	//void RenderManager::ResetCommandBuffers(uint32_t imageIndex) const
	//{
	//	vkResetCommandBuffer(commandBuffers[imageIndex], 0);
	//}

	//void RenderManager::CreateSyncObjects()
	//{
	//	m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	//	m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	//	m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
	//	m_imagesInFlight.resize(m_swapchain->GetSwapchainImageCount(), VK_NULL_HANDLE);

	//	VkSemaphoreCreateInfo semaphoreInfo{};
	//	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	//	VkFenceCreateInfo fenceInfo{};
	//	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	//	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	//	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	//	{
	//		if (vkCreateSemaphore(JoyContext::Graphics->GetDevice(), &semaphoreInfo,
	//		                      JoyContext::Graphics->GetAllocationCallbacks(),
	//		                      &m_imageAvailableSemaphores[i]) != VK_SUCCESS ||
	//			vkCreateSemaphore(JoyContext::Graphics->GetDevice(), &semaphoreInfo,
	//			                  JoyContext::Graphics->GetAllocationCallbacks(),
	//			                  &m_renderFinishedSemaphores[i]) != VK_SUCCESS ||
	//			vkCreateFence(JoyContext::Graphics->GetDevice(), &fenceInfo,
	//			              JoyContext::Graphics->GetAllocationCallbacks(), &m_inFlightFences[i]) != VK_SUCCESS)
	//		{
	//			throw std::runtime_error("failed to create synchronization objects for a frame!");
	//		}
	//	}
	//}

	inline D3D12_RESOURCE_BARRIER Transition(
		_In_ ID3D12Resource* pResource,
		D3D12_RESOURCE_STATES stateBefore,
		D3D12_RESOURCE_STATES stateAfter,
		UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
		D3D12_RESOURCE_BARRIER_FLAGS flags = D3D12_RESOURCE_BARRIER_FLAG_NONE) noexcept
	{
		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = flags;
		barrier.Transition.pResource = pResource;
		barrier.Transition.StateBefore = stateBefore;
		barrier.Transition.StateAfter = stateAfter;
		barrier.Transition.Subresource = subresource;
		return barrier;
	}

	void RenderManager::Update()
	{
		m_queue->ResetForFrame(m_currentFrameIndex);

		const auto commandList = m_queue->GetCommandList();
		// Set necessary state.
		commandList->RSSetViewports(1, &m_viewport);
		commandList->RSSetScissorRects(1, &m_scissorRect);

		// Indicate that the back buffer will be used as a render target.
		D3D12_RESOURCE_BARRIER barrier1 = Transition(
			m_renderTargets[m_currentFrameIndex].resource.Get(),
			D3D12_RESOURCE_STATE_PRESENT,
			D3D12_RESOURCE_STATE_RENDER_TARGET);
		commandList->ResourceBarrier(1, &barrier1);

		commandList->OMSetRenderTargets(1, &m_renderTargets[m_currentFrameIndex].handle, FALSE, nullptr);

		// Record commands.
		const float clearColor[] = {0.0f, 0.2f, 0.4f, 1.0f};
		commandList->ClearRenderTargetView(m_renderTargets[m_currentFrameIndex].handle, clearColor, 0, nullptr);


		ASSERT(m_currentCamera != nullptr);
		glm::mat4 view = m_currentCamera->GetViewMatrix();
		glm::mat4 proj = m_currentCamera->GetProjMatrix();
		for (auto const& sm : m_sharedMaterials)
		{

			commandList->SetPipelineState(sm->GetPipelineObject().Get());
			commandList->SetGraphicsRootSignature(sm->GetRootSignature().Get());
			for (const auto& mr : sm->GetMeshRenderers())
			{
				commandList->SetDescriptorHeaps(2, mr->GetMaterial()->GetHeaps().data());

				commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
				commandList->IASetVertexBuffers(0, 1, mr->GetMesh()->GetVertexBufferView());
				commandList->IASetIndexBuffer(mr->GetMesh()->GetIndexBufferView());

				MVP mvp{
					mr->GetTransform()->GetModelMatrix(),
					view,
					proj
				};
				for (auto param :mr->GetMaterial()->GetRootParams())
				{
					uint32_t index = param.first;
					D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = param.second->GetGPUDescriptorHandleForHeapStart();

					commandList->SetGraphicsRootDescriptorTable(index, gpuHandle);
				}
				commandList->SetGraphicsRoot32BitConstants(2, sizeof(MVP) / 4, &mvp, 0);
				commandList->DrawIndexedInstanced(
					mr->GetMesh()->GetIndexSize(), 
					1, 
					0, 0, 0);

			}
		}

		// Indicate that the back buffer will now be used to present.
		D3D12_RESOURCE_BARRIER barrier2 = Transition(
			m_renderTargets[m_currentFrameIndex].resource.Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT);
		commandList->ResourceBarrier(1, &barrier2);

		ASSERT_SUCC(commandList->Close());

		m_queue->Execute();

		// Present the frame.
		ASSERT_SUCC(m_swapChain->Present(0, 0));

		// Update the frame index.
		m_currentFrameIndex = m_swapChain->GetCurrentBackBufferIndex();

		m_queue->WaitForFence(m_currentFrameIndex);
	}

	//void RenderManager::DrawFrame()
	//{
	//	vkWaitForFences(JoyContext::Graphics->GetDevice(), 1, &m_inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

	//	uint32_t imageIndex;
	//	VkResult result = vkAcquireNextImageKHR(JoyContext::Graphics->GetDevice(),
	//	                                        m_swapchain->GetSwapChain(),
	//	                                        UINT64_MAX,
	//	                                        m_imageAvailableSemaphores[currentFrame],
	//	                                        VK_NULL_HANDLE,
	//	                                        &imageIndex);

	//	if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	//	{
	//		throw std::runtime_error("failed to acquire swap chain image!");
	//	}
	//	m_commonDescriptorSetProvider->UpdateDescriptorSetData(imageIndex);
	//	ResetCommandBuffers(imageIndex);
	//	WriteCommandBuffers(imageIndex);

	//	// Check if a previous frame is using this image (i.e. there is its fence to wait on)
	//	if (m_imagesInFlight[imageIndex] != VK_NULL_HANDLE)
	//	{
	//		vkWaitForFences(JoyContext::Graphics->GetDevice(),
	//		                1,
	//		                &m_imagesInFlight[imageIndex],
	//		                VK_TRUE,
	//		                UINT64_MAX);
	//	}

	//	// Mark the image as now being in use by this frame
	//	m_imagesInFlight[imageIndex] = m_inFlightFences[currentFrame];
	//	vkResetFences(JoyContext::Graphics->GetDevice(), 1, &m_inFlightFences[currentFrame]);

	//	VkSemaphore waitSemaphores[] = {m_imageAvailableSemaphores[currentFrame]};
	//	VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	//	VkSemaphore signalSemaphores[] = {m_renderFinishedSemaphores[currentFrame]};

	//	VkSubmitInfo submitInfo{
	//		VK_STRUCTURE_TYPE_SUBMIT_INFO,
	//		nullptr,
	//		1,
	//		waitSemaphores,
	//		waitStages,
	//		1,
	//		&commandBuffers[imageIndex],
	//		1,
	//		signalSemaphores
	//	};

	//	if (vkQueueSubmit(JoyContext::Graphics->GetGraphicsQueue(), 1, &submitInfo,
	//	                  m_inFlightFences[currentFrame]) != VK_SUCCESS)
	//	{
	//		throw std::runtime_error("failed to submit draw command buffer!");
	//	}

	//	VkSwapchainKHR swapChains[] = {m_swapchain->GetSwapChain()};
	//	VkPresentInfoKHR presentInfo{
	//		VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
	//		nullptr,
	//		1,
	//		signalSemaphores,
	//		1,
	//		swapChains,
	//		&imageIndex,
	//		nullptr
	//	};
	//	result = vkQueuePresentKHR(JoyContext::Graphics->GetPresentQueue(), &presentInfo);
	//	if (result != VK_SUCCESS)
	//	{
	//		throw std::runtime_error("failed to present swap chain image!");
	//	}

	//	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	//}

	//Swapchain* RenderManager::GetSwapchain() const noexcept { return m_swapchain.get(); }

	//VkRenderPass RenderManager::GetMainRenderPass() const noexcept { return m_renderPass->GetRenderPass(); }

	float RenderManager::GetAspect() const noexcept
	{
		return static_cast<float>(m_width) / static_cast<float>(m_height);
	}

	//Texture* RenderManager::GetGBufferPositionTexture() const noexcept
	//{
	//	return m_positionAttachment.get();
	//}

	//Texture* RenderManager::GetGBufferNormalTexture() const noexcept
	//{
	//	return m_normalAttachment.get();
	//}

	//SharedBindingData* RenderManager::GetBindingDataForDefine(uint32_t defineHash) const
	//{
	//	return m_commonDescriptorSetProvider->GetBindingData(defineHash);
	//}
}
