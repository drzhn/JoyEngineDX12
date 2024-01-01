#ifndef MODEL_DATA_SYSTEM_H
#define MODEL_DATA_SYSTEM_H

#include "CommonEngineStructs.h"
#include "IRenderer.h"
#include "ResourceManager/Buffers/DynamicBufferPool.h"

namespace JoyEngine
{
	class TransformProvider
	{
	public:
		TransformProvider():
			m_pool(IRenderer::Get()->GetFrameCount())
		{
		}

		void Init();

		void Update();
		uint32_t Allocate();
		void Free(uint32_t index);

		jmath::mat4x4& GetMatrix(uint32_t transformIndex);
		ResourceView* GetObjectMatricesBufferView(uint32_t frameIndex);

	private:
		DynamicBufferPool<jmath::mat4x4, OBJECT_SIZE> m_pool;
	};
}
#endif // MODEL_DATA_SYSTEM_H
