#ifndef MODEL_DATA_SYSTEM_H
#define MODEL_DATA_SYSTEM_H

#include <glm/fwd.hpp>

#include "CommonEngineStructs.h"
#include "ResourceManager/Buffers/DynamicBufferPool.h"

namespace JoyEngine
{
	class WorldMatricesProvider
	{
	public:
		WorldMatricesProvider() = delete;

		explicit WorldMatricesProvider(uint32_t frameCount)
			:m_pool(frameCount)
		{

		}

		void Update();
		uint32_t Allocate();
		void Free(uint32_t index); // TODO
	private:
		DynamicBufferPool<glm::mat4, ObjectMatricesData, OBJECT_SIZE> m_pool;
	};
}
#endif // MODEL_DATA_SYSTEM_H
