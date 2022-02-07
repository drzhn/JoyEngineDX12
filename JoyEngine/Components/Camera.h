#ifndef CAMERA_H
#define CAMERA_H

#include "Component.h"
#include <glm/glm.hpp>

#include "Common/CameraUnit.h"

namespace JoyEngine
{
	class Camera : public Component
	{
	public:
		void Enable() override;
		void Disable() override;
		void Update() override;
		[[nodiscard]] glm::mat4x4 GetViewMatrix() const;
		[[nodiscard]] glm::mat4x4 GetProjMatrix() const;
	private:
		CameraUnit m_cameraUnit;
	};
}

#endif //CAMERA_H
