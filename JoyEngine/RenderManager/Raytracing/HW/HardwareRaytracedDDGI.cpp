#include "HardwareRaytracedDDGI.h"

#include "ResourceManager/ResourceManager.h"

namespace JoyEngine
{
	HardwareRaytracedDDGI::HardwareRaytracedDDGI()
	{
		m_raytracingPipeline = std::make_unique<RaytracingPipeline>(RaytracingPipelineArgs{
			GUID::StringToGuid("b2597599-94ef-43ed-abd8-46d3adbb75d4")
		});
	}
}
