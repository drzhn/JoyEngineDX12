#include "Transform.h"

#include "RenderManager/TransformProvider.h"
#include "SceneManager/GameObject.h"

namespace JoyEngine
{
	Transform::Transform(GameObject& gameObject, uint32_t transformIndex, TransformProvider& transformProvider) :
		Transform(
			gameObject,
			transformIndex,
			transformProvider,
			jmath::vec3(0.f, 0.f, 0.f),
			jmath::vec3(0.f, 0.f, 0.f),
			jmath::vec3(1.f, 1.f, 1.f))
	{
	}

	Transform::Transform(GameObject& gameObject, uint32_t transformIndex, TransformProvider& transformProvider, jmath::vec3 pos, jmath::vec3 rot, jmath::vec3 scale):
		m_gameObject(gameObject),
		m_transformIndex(transformIndex),
		m_transformProvider(transformProvider)
	{
		SetPosition(pos);
		SetRotation(rot);
		SetScale(scale);
	}

	void Transform::SetPosition(jmath::vec3 pos) noexcept
	{
		SetXPosition(jmath::loadPosition(&pos));
	}


	void Transform::SetRotation(jmath::vec3 rot) noexcept
	{
		SetRotation(jmath::eulerToQuat(jmath::vec3(
			jmath::toRadians(rot.x),
			jmath::toRadians(rot.y),
			jmath::toRadians(rot.z)
		)));
	}

	void Transform::SetRotation(jmath::quat rot) noexcept
	{
		m_localRotation = rot;
		UpdateMatrix();
	}

	void Transform::SetScale(jmath::vec3 scale) noexcept
	{
		SetXScale(jmath::loadPosition(&scale));
	}

	void Transform::SetXPosition(jmath::xvec4 pos) noexcept
	{
		m_localPosition = pos;
		UpdateMatrix();
	}

	void Transform::SetXScale(jmath::xvec4 scale) noexcept
	{
		m_localScale = scale;
		UpdateMatrix();
	}

	void Transform::UpdateChildrenMatrix(GameObject* object)
	{
		if (object == nullptr) return;

		object->GetTransform().UpdateThisTransformMatrix();

		UpdateChildrenMatrix(object->GetNextSibling());
		UpdateChildrenMatrix(object->GetFirstChild());
	}

	void Transform::UpdateThisTransformMatrix() const
	{
		jmath::mat4x4 mat = jmath::trs(m_localPosition, m_localRotation, m_localScale);

		if (m_gameObject.GetParent() != nullptr)
		{
			mat = jmath::mul(
				mat,
				m_transformProvider.GetMatrix(m_gameObject.GetParent()->GetTransform().m_transformIndex)
			);
		}

		m_transformProvider.GetMatrix(m_gameObject.GetTransform().GetTransformIndex()) = mat;
	}

	void Transform::UpdateMatrix() const
	{
		UpdateThisTransformMatrix();

		UpdateChildrenMatrix(m_gameObject.GetFirstChild());
	}

	const jmath::mat4x4& Transform::GetModelMatrix() const noexcept
	{
		return m_transformProvider.GetMatrix(m_transformIndex);
	}

	jmath::vec3 Transform::GetPosition() const noexcept { return jmath::toVec3(m_localPosition); }

	jmath::quat Transform::GetRotation() const noexcept { return m_localRotation; }

	jmath::vec3 Transform::GetScale() const noexcept { return jmath::toVec3(m_localScale); }

	jmath::vec3 Transform::GetForward() const noexcept { return jmath::toVec3(GetXForward()); }

	jmath::vec3 Transform::GetUp() const noexcept { return jmath::toVec3(GetXUp()); }

	jmath::vec3 Transform::GetRight() const noexcept { return jmath::toVec3(GetXRight()); }

	jmath::xvec4 Transform::GetXPosition() const noexcept { return m_localPosition; }

	jmath::xvec4 Transform::GetXScale() const noexcept { return m_localScale; }

	jmath::xvec4 Transform::GetXForward() const noexcept { return jmath::mul(GetModelMatrix(), jmath::xforward); }

	jmath::xvec4 Transform::GetXUp() const noexcept { return jmath::mul(GetModelMatrix(), jmath::xup); }

	jmath::xvec4 Transform::GetXRight() const noexcept { return jmath::mul(GetModelMatrix(), jmath::xright); }
}
