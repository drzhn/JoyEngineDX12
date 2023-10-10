#include "CameraUnit.h"

#include "Math/MathTypes.h"

#include "Utils/Assert.h"

namespace JoyEngine
{
	CameraUnit::CameraUnit(float aspect, float width, float height, float fovDeg, float nearPlane, float farPlane):
		m_type(CameraUnitType::Perspective),
		m_aspect(aspect),
		m_width(width),
		m_height(height),
		m_fovRad(jmath::toRadians(fovDeg)),
		m_near(nearPlane),
		m_far(farPlane)
	{
		jmath::vec2 a;
	}

	CameraUnit::CameraUnit(float aspect, float size, float nearPlane, float farPlane):
		m_type(CameraUnitType::Orthographic),
		m_aspect(aspect),
		m_size(size),
		m_near(nearPlane),
		m_far(farPlane)
	{
	}

	jmath::mat4x4 CameraUnit::GetProjMatrix() const
	{
		switch (m_type)
		{
		case CameraUnitType::Perspective:
			return jmath::perspectiveFovLH_ZO(m_fovRad, m_width, m_height, m_near, m_far);
		case CameraUnitType::Orthographic:
			return jmath::orthoLH_ZO(-m_size * m_aspect, m_size * m_aspect, -m_size, m_size, m_near, m_far);
		}
		ASSERT(false);
		return jmath::identity();
	}

	jmath::mat4x4 CameraUnit::GetViewMatrix(const jmath::xvec4 position, const jmath::quat rotation) const
	{
		const jmath::xvec4 focus = position + jmath::rotate3(jmath::xforward, rotation);
		const jmath::xvec4 up = jmath::rotate3(jmath::xup, rotation);

		return jmath::lookAtLH(position, focus, up);
	}
}
