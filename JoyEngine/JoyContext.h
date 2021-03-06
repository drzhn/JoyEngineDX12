#ifndef JOY_CONTEXT_H
#define JOY_CONTEXT_H

namespace JoyEngine
{
	class InputManager;
	class GraphicsManager;
	class MemoryManager;
	class DataManager;
	class DescriptorManager;
	class ResourceManager;
	class SceneManager;
	class RenderManager;
	class EngineMaterialProvider;

	class JoyContext
	{
	public:
		static void Init(
			InputManager* inputManager,
			GraphicsManager* graphicsContext,
			MemoryManager* memoryManager,
			DataManager* dataManager,
			DescriptorManager* descriptorManager,
			ResourceManager* resourceManager,
			SceneManager* sceneManager,
			RenderManager* renderManager,
			EngineMaterialProvider* engineMaterialProvider
		);

		static InputManager* Input;
		static GraphicsManager* Graphics;
		static MemoryManager* Memory;
		static DataManager* Data;
		static DescriptorManager* Descriptors;
		static ResourceManager* Resource;
		static SceneManager* Scene;
		static RenderManager* Render;
		static EngineMaterialProvider* EngineMaterials;
	};
}

#endif //JOY_CONTEXT_H
