#ifndef LIGHT_H
#define LIGHT_H

#include <glm/glm.hpp>

#include "Common/Color.h"

namespace JoyEngine
{
	enum LightType
	{
		Point = 0,
		Spot = 1
	};

	class Light
	{
	public:

	protected:
		float m_intensity;
		Color m_color;
	};

	class PointLight : public Light
	{
	public:
	private:
		float m_radius;
	};

	class SpotLight : public Light
	{
	public:
	private:
		glm::vec3 m_direction;
		float m_size;
		float m_angle;
	};
}

#endif // LIGHT_H
