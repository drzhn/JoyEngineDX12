#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp> 
#include <glm/gtx/quaternion.hpp>


namespace JoyEngine
{

	class Transform {
	public:
		Transform();

		Transform(glm::vec3 pos, glm::vec3 rot, glm::vec3 scale);

		void SetPosition(glm::vec3 pos) noexcept;

		[[nodiscard]] glm::vec3 GetPosition() const noexcept;

		void SetRotation(glm::vec3 rot) noexcept;
		void SetRotation(glm::quat rot) noexcept;

		[[nodiscard]] glm::quat GetRotation() const noexcept;

		void SetScale(glm::vec3 scale) noexcept;

		[[nodiscard]] glm::vec3 GetScale() const noexcept;

		[[nodiscard]] glm::mat4 GetModelMatrix() const;

	private:
		void UpdateModelMatrix();
	private:
		glm::vec3 m_localPosition;
		glm::quat m_localRotation;
		glm::vec3 m_localScale;

		glm::mat4 m_modelMatrix;
	};
}

#endif //TRANSFORM_H
