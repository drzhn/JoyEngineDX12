#ifndef LIGHT_SYSTEM_H
#define LIGHT_SYSTEM_H

struct DirectionalLightInfo;

namespace JoyEngine
{
	class LightBase;

	class ILightSystem
	{
	public:
		virtual ~ILightSystem() = default;
		virtual void Update(const uint32_t frameIndex) = 0;

		virtual DirectionalLightInfo& GetDirectionalLightData() =0;
		virtual uint32_t RegisterDirectionalLight() = 0;
		virtual void UnregisterDirectionalLight() = 0;

		virtual uint32_t RegisterLight(LightBase* light) = 0;
		virtual void UnregisterLight(LightBase* light) = 0;
		virtual LightInfo& GetLightInfo(uint32_t lightIndex) = 0;
	};
}
#endif // LIGHT_SYSTEM_H
