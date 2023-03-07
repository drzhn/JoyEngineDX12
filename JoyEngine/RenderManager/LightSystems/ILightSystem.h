#ifndef LIGHT_SYSTEM_H
#define LIGHT_SYSTEM_H

namespace JoyEngine
{
	class DirectionalLight;

	class ILightSystem
	{
	public:
		virtual void Update() = 0;

		virtual void RegisterDirectionalLight(DirectionalLight* light) = 0;
		virtual void UnregisterDirectionalLight(DirectionalLight* light) = 0;
	};
}
#endif // LIGHT_SYSTEM_H
