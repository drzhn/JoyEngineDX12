#include "JoyContext.h"

namespace JoyEngine
{
	InputManager* JoyContext::Input = nullptr;

	GraphicsManager* JoyContext::Graphics = nullptr;

	MemoryManager* JoyContext::Memory = nullptr;

	DataManager* JoyContext::Data = nullptr;

	DescriptorManager* JoyContext::Descriptors = nullptr;

	ResourceManager* JoyContext::Resource = nullptr;

	SceneManager* JoyContext::Scene = nullptr;

	RenderManager* JoyContext::Render = nullptr;

	EngineMaterialProvider* JoyContext::EngineMaterials = nullptr;

	void JoyContext::Init(
		InputManager* inputManager, 
		GraphicsManager* graphicsContext, 
		MemoryManager* memoryManager, 
		DataManager* dataManager,
		DescriptorManager* descriptorManager, 
		ResourceManager* resourceManager, 
		SceneManager* sceneManager,
		RenderManager* renderManager,
		EngineMaterialProvider* engineMaterialProvider
	)
	{
		Input = inputManager;
		Graphics = graphicsContext;
		Memory = memoryManager;
		Data = dataManager;
		Descriptors = descriptorManager;
		Resource = resourceManager;
		Scene = sceneManager;
		Render = renderManager;
		EngineMaterials = engineMaterialProvider;
		
	}
}
