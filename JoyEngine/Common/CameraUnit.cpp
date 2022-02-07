#include "CameraUnit.h"

#include <glm/detail/type_quat.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

namespace JoyEngine
{
	CameraUnit::CameraUnit(
		float aspect,
		float width,
		float height,
		float fov,
		float near,
		float far
	):
		m_aspect(aspect),
		m_width(width),
		m_height(height),
		m_fov(fov),
		m_near(near),
		m_far(far)

	{
	}

	glm::mat4x4 CameraUnit::GetProjMatrix() const
	{
		const glm::mat4 proj = glm::perspectiveFovLH_ZO(glm::radians(m_fov), m_width, m_height, m_near, m_far);
		return proj;
	}

	glm::mat4x4 CameraUnit::GetViewMatrix(glm::vec3 position, glm::quat rotation) const
	{
		const glm::vec3 eye = position;
		const glm::vec3 center = position + rotation * glm::vec3(0, 0, 1);
		const glm::vec3 up = rotation * glm::vec3(0, 1, 0);

		return glm::lookAtLH(eye, center, up);
	}
}
