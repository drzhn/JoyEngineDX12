#include "Transform.h"

namespace JoyEngine
{
	Transform::Transform() : Transform(
		jmath::vec3(0.f, 0.f, 0.f),
		jmath::vec3(0.f, 0.f, 0.f),
		jmath::vec3(1.f, 1.f, 1.f))
	{
	}

	Transform::Transform(jmath::vec3 pos, jmath::vec3 rot, jmath::vec3 scale)
	{
		SetPosition(pos);
		SetRotation(rot);
		SetScale(scale);
	}

	void Transform::SetPosition(jmath::vec3 pos) noexcept
	{
		m_localPosition = jmath::loadPosition(&pos);
	}


	void Transform::SetRotation(jmath::vec3 rot) noexcept
	{
		m_localRotation = jmath::eulerToQuat(jmath::vec3(
			jmath::toRadians(rot.x),
			jmath::toRadians(rot.y),
			jmath::toRadians(rot.z)
		));
	}

	void Transform::SetRotation(jmath::quat rot) noexcept
	{
		m_localRotation = rot;
	}

	void Transform::SetScale(jmath::vec3 scale) noexcept
	{
		m_localScale = jmath::loadPosition(&scale);
	}

	void Transform::SetXPosition(jmath::xvec4 pos) noexcept
	{
		m_localPosition = pos;
	}

	void Transform::SetXScale(jmath::xvec4 scale) noexcept
	{
		m_localScale = scale;
	}

	jmath::vec3 Transform::GetPosition() const noexcept
	{
		return jmath::toVec3(m_localPosition);
	}

	jmath::quat Transform::GetRotation() const noexcept { return m_localRotation; }

	jmath::vec3 Transform::GetScale() const noexcept { return jmath::toVec3(m_localScale); }

	jmath::mat4x4 Transform::GetModelMatrix() const noexcept
	{
		return jmath::trs(m_localPosition, m_localRotation, m_localScale);
	}

	jmath::vec3 Transform::GetForward() const noexcept
	{
		return jmath::toVec3(GetXForward()); // TODO OPTIMIZE THIS!!!!
	}

	jmath::vec3 Transform::GetUp() const noexcept
	{
		return jmath::toVec3(GetXUp());
	}

	jmath::vec3 Transform::GetRight() const noexcept
	{
		return jmath::toVec3(GetXRight());
	}

	jmath::xvec4 Transform::GetXPosition() const noexcept
	{
		return m_localPosition;
	}

	jmath::xvec4 Transform::GetXScale() const noexcept
	{
		return m_localScale;
	}

	jmath::xvec4 Transform::GetXForward() const noexcept
	{
		return jmath::mul(GetModelMatrix(), jmath::xforward);
	}

	jmath::xvec4 Transform::GetXUp() const noexcept
	{
		return jmath::mul(GetModelMatrix(), jmath::xup);
	}

	jmath::xvec4 Transform::GetXRight() const noexcept
	{
		return jmath::mul(GetModelMatrix(), jmath::xright);
	}
}
