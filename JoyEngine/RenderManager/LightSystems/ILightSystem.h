#ifndef LIGHT_SYSTEM_H
#define LIGHT_SYSTEM_H

struct DirectionalLightData;

namespace JoyEngine
{
	class ILightSystem
	{
	public:
		virtual ~ILightSystem() = default;
		virtual void Update(const uint32_t frameIndex) = 0;

		virtual DirectionalLightData& GetDirectionalLightData() =0;
		virtual uint32_t RegisterDirectionalLight() = 0;
		virtual void UnregisterDirectionalLight() = 0;
	};
}
#endif // LIGHT_SYSTEM_H
