#ifndef IRENDERMANAGER_H
#define IRENDERMANAGER_H

namespace JoyEngine
{
	class TransformProvider;
	class Camera;
	class DirectionalLight;
	class SharedMaterial;

	class IRenderManager
	{
	public:
		virtual ~IRenderManager() = default;
		virtual void RegisterSharedMaterial(SharedMaterial*) = 0;
		virtual void UnregisterSharedMaterial(SharedMaterial*) = 0;
		//virtual void RegisterLight(Light*) = 0;
		//virtual void UnregisterLight(Light*) = 0;
		virtual void RegisterDirectionLight(DirectionalLight*) = 0;
		virtual void UnregisterDirectionLight(DirectionalLight*) = 0;
		virtual void RegisterCamera(Camera* camera) = 0;
		virtual void UnregisterCamera(Camera* camera) = 0;

		[[nodiscard]] virtual TransformProvider* GetTransformProvider() const noexcept = 0;
		[[nodiscard]] virtual float GetAspect() const noexcept = 0;
		[[nodiscard]] virtual float GetWidth_f() const noexcept = 0;
		[[nodiscard]] virtual float GetHeight_f() const noexcept = 0;
		[[nodiscard]] virtual uint32_t GetWidth() const noexcept = 0;
		[[nodiscard]] virtual uint32_t GetHeight() const noexcept = 0;
		[[nodiscard]] virtual uint32_t GetFrameCount() const noexcept = 0;
	};
}
#endif // IRENDERMANAGER_H
