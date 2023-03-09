#ifndef CAMERA_H
#define CAMERA_H

#include "Component.h"
#include <glm/glm.hpp>

#include "Common/CameraUnit.h"

namespace JoyEngine
{
	class IRenderManager;

	class Camera : public Component
	{
	public:
		Camera() = delete;
		explicit Camera(GameObject& go, IRenderManager* manager, float cameraNear, float cameraFar, float cameraFov);
		void Enable() override;
		void Disable() override;
		void Update() override;
		[[nodiscard]] glm::mat4x4 GetViewMatrix() const;
		[[nodiscard]] glm::mat4x4 GetProjMatrix() const;

		[[nodiscard]] float GetFovRadians() const;
		[[nodiscard]] float GetNear() const;
		[[nodiscard]] float GetFar() const;
	private:
		CameraUnit m_cameraUnit;
		IRenderManager* m_manager;
	};
}

#endif //CAMERA_H
