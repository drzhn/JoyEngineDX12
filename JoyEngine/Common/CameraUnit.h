#ifndef CAMERA_UNIT_H
#define CAMERA_UNIT_H

#include <glm/glm.hpp>

namespace JoyEngine
{
	enum class CameraUnitType
	{
		Perspective,
		Orthographic
	};

	class CameraUnit
	{
	public:
		CameraUnit() = default;
		virtual ~CameraUnit() = default;

		explicit CameraUnit(float aspect, float width, float height, float fovDeg, float nearPlane, float farPlane);
		explicit CameraUnit(float aspect, float size, float nearPlane, float farPlane);

		[[nodiscard]] virtual glm::mat4x4 GetProjMatrix() const;
		[[nodiscard]] virtual glm::mat4x4 GetViewMatrix(glm::vec3 position, glm::quat rotation) const;

		[[nodiscard]] float GetNear() const noexcept { return m_near; }
		[[nodiscard]] float GetFar() const noexcept { return m_far; }
		[[nodiscard]] float GetFOVRadians() const noexcept { return m_fovRad; }
		[[nodiscard]] float GetAspect() const noexcept { return m_aspect; }

	protected:
		CameraUnitType m_type;

		float m_aspect = 0;
		float m_width = 0;
		float m_height = 0;
		float m_fovRad = 0;
		float m_size = 0;
		float m_near = 0;
		float m_far = 0;
	};
}

#endif // CAMERA_UNIT_H
