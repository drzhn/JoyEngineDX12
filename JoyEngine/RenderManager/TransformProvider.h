#ifndef MODEL_DATA_SYSTEM_H
#define MODEL_DATA_SYSTEM_H

#include "CommonEngineStructs.h"
#include "ResourceManager/Buffers/DynamicBufferPool.h"
#include "SceneManager/Transform.h"

namespace JoyEngine
{
	class TransformProvider
	{
	public:
		TransformProvider() = delete;

		explicit TransformProvider(uint32_t frameCount)
			:m_pool(frameCount)
		{

		}

		void Update(uint32_t frameIndex);
		uint32_t Allocate();
		void Free(uint32_t index);

		Transform& GetTransform(uint32_t transformIndex);
		ResourceView* GetObjectMatricesBufferView(uint32_t frameIndex);

	private:
		DynamicBufferPool<Transform, ObjectMatricesData, OBJECT_SIZE> m_pool;
	};
}
#endif // MODEL_DATA_SYSTEM_H
