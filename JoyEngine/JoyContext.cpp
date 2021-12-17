#include "JoyContext.h"

namespace JoyEngine
{
	InputManager* JoyContext::Input = nullptr;

	GraphicsManager* JoyContext::Graphics = nullptr;

	//MemoryManager* JoyContext::Memory = nullptr;

	//DataManager* JoyContext::Data = nullptr;

	//DescriptorSetManager* JoyContext::DescriptorSet = nullptr;

	//ResourceManager* JoyContext::Resource = nullptr;

	//SceneManager* JoyContext::Scene = nullptr;

	RenderManager* JoyContext::Render = nullptr;

	void JoyContext::Init(
		InputManager* inputManager, 
		GraphicsManager* graphicsContext, 
		//MemoryManager* memoryManager, 
		//DataManager* dataManager,
		//DescriptorSetManager* descriptorSetManager, 
		//ResourceManager* resourceManager, 
		//SceneManager* sceneManager,
		RenderManager* renderManager)
	{
		Input = inputManager;
		Graphics = graphicsContext;
		//Memory = memoryManager;
		//Data = dataManager;
		//DescriptorSet = descriptorSetManager;
		//Resource = resourceManager;
		//Scene = sceneManager;
		Render = renderManager;
		
	}
}
