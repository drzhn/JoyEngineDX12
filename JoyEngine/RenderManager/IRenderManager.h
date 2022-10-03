#ifndef IRENDERMANAGER_H
#define IRENDERMANAGER_H

namespace JoyEngine
{
	class Camera;
	class Light;
	class SharedMaterial;

	class IRenderManager
	{
	public:
		virtual ~IRenderManager() = default;
		virtual void RegisterSharedMaterial(SharedMaterial*) = 0;
		virtual void UnregisterSharedMaterial(SharedMaterial*) = 0;
		virtual void RegisterLight(Light*) = 0;
		virtual void UnregisterLight(Light*) = 0;
		virtual void RegisterDirectionLight(Light*) = 0;
		virtual void UnregisterDirectionLight(Light*) = 0;
		virtual void RegisterCamera(Camera* camera) = 0;
		virtual void UnregisterCamera(Camera* camera) = 0;
		virtual float GetAspect() const noexcept = 0;
		virtual float GetWidth() const noexcept = 0;
		virtual float GetHeight() const noexcept = 0;
	};
}
#endif // IRENDERMANAGER_H
