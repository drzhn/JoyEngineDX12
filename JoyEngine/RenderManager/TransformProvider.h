#ifndef MODEL_DATA_SYSTEM_H
#define MODEL_DATA_SYSTEM_H

#include "CommonEngineStructs.h"
#include "IRenderManager.h"
#include "ResourceManager/Buffers/DynamicBufferPool.h"

namespace JoyEngine
{
	class TransformProvider
	{
	public:
		TransformProvider() = delete;

		explicit TransformProvider(const IRenderManager* renderManager):
			m_renderManager(renderManager),
			m_pool(m_renderManager->GetFrameCount())
		{
		}

		void Update(uint32_t frameIndex);
		uint32_t Allocate();
		void Free(uint32_t index);

		jmath::mat4x4& GetMatrix(uint32_t transformIndex);
		ResourceView* GetObjectMatricesBufferView(uint32_t frameIndex);

	private:
		const IRenderManager* m_renderManager;
		DynamicBufferPool<jmath::mat4x4, OBJECT_SIZE> m_pool;
	};
}
#endif // MODEL_DATA_SYSTEM_H
