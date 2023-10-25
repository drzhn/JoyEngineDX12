#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "Common/Math/MathTypes.h"
#include "RenderManager/TransformProvider.h"

namespace JoyEngine
{
	class TransformProvider;

	class Transform
	{
	public:
		Transform() = delete;

		explicit Transform(uint32_t transformIndex, TransformProvider& transformProvider);
		explicit Transform(uint32_t transformIndex, TransformProvider& transformProvider, jmath::vec3 pos, jmath::vec3 rot, jmath::vec3 scale);

		[[nodiscard]] uint32_t GetTransformIndex() const noexcept { return m_transformIndex; }
		[[nodiscard]] uint32_t const* GetTransformIndexPtr() const noexcept { return &m_transformIndex; }

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

		void UpdateMatrix();
		[[nodiscard]] const jmath::mat4x4& GetModelMatrix() const noexcept
		{
			return m_transformProvider.GetMatrix(m_transformIndex);
		}

	private:
		jmath::xvec4 m_localPosition;
		jmath::quat m_localRotation;
		jmath::xvec4 m_localScale;

		const uint32_t m_transformIndex;

		// There is no scene object without Transform and no Transform without the scene object
		// I moved all math to separate class just because of nicer code style 
		TransformProvider& m_transformProvider;
	};
}

#endif //TRANSFORM_H
