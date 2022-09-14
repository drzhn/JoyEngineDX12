#include "Raytracing.h"

namespace JoyEngine
{
	Raytracing::Raytracing()
	{
		m_testBuffer = std::make_unique<DataBuffer<uint32_t, 100>>(42);
		m_testBuffer->GetLocalData()[0] = 0;
		m_testBuffer->ReadbackGpuData();
		OutputDebugStringA(std::to_string(m_testBuffer->GetLocalData()[0]).c_str());
	}
}
