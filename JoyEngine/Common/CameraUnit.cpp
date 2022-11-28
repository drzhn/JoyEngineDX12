#include "CameraUnit.h"

#include <glm/detail/type_quat.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "Utils/Assert.h"

namespace JoyEngine
{
	CameraUnit::CameraUnit(float aspect, float width, float height, float fov, float nearPlane, float farPlane):
		m_type(CameraUnitType::Perspective),
		m_aspect(aspect),
		m_width(width),
		m_height(height),
		m_fov(fov),
		m_near(nearPlane),
		m_far(farPlane)
	{
	}

	CameraUnit::CameraUnit(float aspect, float size, float nearPlane, float farPlane):
		m_type(CameraUnitType::Orthographic),
		m_aspect(aspect),
		m_size(size),
		m_near(nearPlane),
		m_far(farPlane)
	{
	}

	glm::mat4x4 CameraUnit::GetProjMatrix() const
	{
		switch (m_type)
		{
		case CameraUnitType::Perspective:
			return glm::perspectiveFovLH_ZO(glm::radians(m_fov), m_width, m_height, m_near, m_far);
		case CameraUnitType::Orthographic:
			return glm::orthoLH_ZO(-m_size * m_aspect, m_size * m_aspect, -m_size, m_size, m_near, m_far);
		}
		ASSERT(false);
		return glm::identity<glm::mat4>();
	}

	glm::mat4x4 CameraUnit::GetViewMatrix(glm::vec3 position, glm::quat rotation) const
	{
		const glm::vec3 eye = position;
		const glm::vec3 center = position + rotation * glm::vec3(0, 0, 1);
		const glm::vec3 up = rotation * glm::vec3(0, 1, 0);

		return glm::lookAtLH(eye, center, up);
	}
}
