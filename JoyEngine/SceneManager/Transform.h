#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "Common/Math/MathTypes.h"

namespace JoyEngine
{
	class Transform
	{
	public:
		Transform();

		Transform(jmath::vec3 pos, jmath::vec3 rot, jmath::vec3 scale);

		void SetRotation(jmath::vec3 rot) noexcept;
		void SetRotation(jmath::quat rot) noexcept;
		void SetPosition(jmath::vec3 pos) noexcept;
		void SetScale(jmath::vec3 scale) noexcept;

		void SetXPosition(jmath::xvec4 pos) noexcept;
		void SetXScale(jmath::xvec4 scale) noexcept;

		[[nodiscard]] jmath::quat GetRotation() const noexcept;

		[[nodiscard]] jmath::vec3 GetPosition() const noexcept;
		[[nodiscard]] jmath::vec3 GetScale() const noexcept;

		[[nodiscard]] jmath::vec3 GetForward() const noexcept;
		[[nodiscard]] jmath::vec3 GetUp() const noexcept;
		[[nodiscard]] jmath::vec3 GetRight() const noexcept;

		[[nodiscard]] jmath::xvec4 GetXPosition() const noexcept;
		[[nodiscard]] jmath::xvec4 GetXScale() const noexcept;
		[[nodiscard]] jmath::xvec4 GetXForward() const noexcept;
		[[nodiscard]] jmath::xvec4 GetXUp() const noexcept;
		[[nodiscard]] jmath::xvec4 GetXRight() const noexcept;


		[[nodiscard]] jmath::mat4x4 GetModelMatrix() const noexcept;
	private:
		jmath::xvec4 m_localPosition;
		jmath::quat m_localRotation;
		jmath::xvec4 m_localScale;
	};
}

#endif //TRANSFORM_H
