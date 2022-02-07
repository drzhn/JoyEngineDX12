#ifndef CAMERA_UNIT_H
#define CAMERA_UNIT_H

#include <glm/glm.hpp>

namespace JoyEngine
{
	class CameraUnit 
	{
	public:
		CameraUnit() = default;
		~CameraUnit() = default;

		explicit CameraUnit(float aspect, float width, float height, float fov, float near, float far);
		[[nodiscard]] virtual glm::mat4x4 GetProjMatrix() const;
		[[nodiscard]] virtual glm::mat4x4 GetViewMatrix(glm::vec3 position, glm::quat rotation) const;
	protected:
		float m_aspect;
		float m_width;
		float m_height;
		float m_fov;
		float m_near;
		float m_far;
	};
}

#endif // CAMERA_UNIT_H
