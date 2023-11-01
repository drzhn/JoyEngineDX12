#ifndef VECTOR_H
#define VECTOR_H

#include <utility>

#include "directxmath.h"

namespace JoyEngine
{
	namespace jmath
	{
		template <typename T>
		struct PODVector : public T
		{
			template <typename... Args>
			PODVector(Args&&... args) :
				T(std::forward<Args>(args)...)
			{
			}

			float operator[](int i) const
			{
				return *(reinterpret_cast<const float*>(this) + i);
			}

			PODVector<T> operator+(const PODVector<T>& a) const
			{
				T result;
				for (int i = 0; i < sizeof(T) / sizeof(float); i++)
				{
					*(reinterpret_cast<float*>(&result) + i) =
						this->operator[](i) + a[i];
				}
				return result;
			}

			PODVector<T> operator*(const PODVector<T>& a) const
			{
				T result;
				for (int i = 0; i < sizeof(T) / sizeof(float); i++)
				{
					*(reinterpret_cast<float*>(&result) + i) =
						this->operator[](i) * a[i];
				}
				return result;
			}

			PODVector<T> operator*(float val) const
			{
				T result;
				for (int i = 0; i < sizeof(T) / sizeof(float); i++)
				{
					*(reinterpret_cast<float*>(&result) + i) =
						this->operator[](i) * val;
				}
				return result;
			}
		};

		typedef PODVector<DirectX::XMFLOAT2> vec2;
		typedef PODVector<DirectX::XMFLOAT3> vec3;
		typedef PODVector<DirectX::XMFLOAT4> vec4;

		typedef DirectX::XMINT2 uvec2;
		typedef DirectX::XMINT3 uvec3;

		typedef DirectX::XMMATRIX mat4x4;
		typedef DirectX::XMVECTOR xvec4;
		typedef DirectX::XMVECTOR quat;

		float toRadians(float degree);

		mat4x4 perspectiveFovLH_ZO(float fovRadians, float width, float height, float nearPlane, float farPlane);
		mat4x4 orthoLH_ZO(float left, float right, float bottom, float top, float nearPlane, float farPlane);
		mat4x4 lookAtLH(xvec4 eye, xvec4 focus, xvec4 up);
		mat4x4 identity();
		mat4x4 trs(const xvec4& translation, const quat& rotation, const xvec4& scale);
		xvec4 mul(const mat4x4& matrix, const xvec4& v);
		xvec4 mul(const float f, const xvec4& v);
		mat4x4 mul(const mat4x4& matrix1, const mat4x4& matrix2);
		quat mul(const quat& a, const quat& b);
		mat4x4 inverse(const mat4x4&);

		xvec4 loadPosition(const vec3& position);
		xvec4 loadRotation(const vec3& rotation);
		xvec4 loadVec4(const vec4& v);
		vec3 toVec3(xvec4 v);

		xvec4 rotate3(xvec4 v, quat q);
		quat eulerToQuat(const vec3& euler);
		xvec4 angleAxis(xvec4 axis, float angle);

		float min(float a, float b);
		vec3 min(const vec3& a, const vec3& b);

		float max(float a, float b);
		vec3 max(const vec3& a, const vec3& b);

		float sin(float radians);
		float cos(float radians);

		inline xvec4 xup{0, 1, 0, 0};
		inline xvec4 xforward{0, 0, 1, 0};
		inline xvec4 xright{1, 0, 0, 0};
		inline xvec4 xone{1, 1, 1, 1};
		inline xvec4 xzero{0, 0, 0, 0};
	}

	jmath::vec2 operator+(const jmath::vec2&, const jmath::vec2&);
	jmath::xvec4 operator+(const jmath::xvec4&, const jmath::xvec4&);
}
#endif // VECTOR_H
