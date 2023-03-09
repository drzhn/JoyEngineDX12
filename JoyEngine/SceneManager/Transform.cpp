#include "Transform.h"

namespace JoyEngine
{
	Transform::Transform() : Transform(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), glm::vec3(1, 1, 1))
	{
	}

	Transform::Transform(glm::vec3 pos, glm::vec3 rot, glm::vec3 scale)
	{
		SetPosition(pos);
		SetRotation(rot);
		SetScale(scale);
	}

	void Transform::SetPosition(glm::vec3 pos) noexcept
	{
		m_localPosition = pos;
	}


	void Transform::SetRotation(glm::vec3 rot) noexcept
	{
		m_localRotation = glm::quat(glm::vec3(
			glm::radians(rot.x),
			glm::radians(rot.y),
			glm::radians(rot.z)
		));
	}

	void Transform::SetRotation(glm::quat rot) noexcept
	{
		m_localRotation = rot;
	}

	void Transform::SetScale(glm::vec3 scale) noexcept
	{
		m_localScale = scale;
	}

	glm::vec3 Transform::GetPosition() const noexcept { return m_localPosition; }

	glm::quat Transform::GetRotation() const noexcept { return m_localRotation; }

	glm::vec3 Transform::GetEulerRotation() const noexcept
	{
		return glm::eulerAngles(m_localRotation);
	}

	glm::vec3 Transform::GetScale() const noexcept { return m_localScale; }

	glm::mat4 Transform::GetModelMatrix() const noexcept
	{
		const glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), m_localPosition);
		const glm::mat4 rotationMatrix = glm::toMat4(m_localRotation);
		const glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), m_localScale);

		return translationMatrix * rotationMatrix * scaleMatrix;
	}

	glm::vec3 Transform::GetForward() const noexcept
	{
		return GetModelMatrix() * glm::vec4(0, 0, 1, 0); // TODO OPTIMIZE THIS!!!!
	}

	glm::vec3 Transform::GetUp() const noexcept
	{
		return GetModelMatrix() * glm::vec4(0, 1, 0, 0);
	}

	glm::vec3 Transform::GetRight() const noexcept
	{
		return GetModelMatrix() * glm::vec4(1, 0, 0, 0);
	}
}
