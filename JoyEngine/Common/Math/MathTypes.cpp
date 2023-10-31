#include "MathTypes.h"


namespace JoyEngine
{
	namespace jmath
	{
		DirectX::XMFLOAT2 operator+(const DirectX::XMFLOAT2& v1, const DirectX::XMFLOAT2& v2)
		{
			return {v1.x + v2.x, v1.y + v2.y};
		}

		float flt3::operator[](int i) const
		{
			return *(&x + i);
		}

		jmath::flt3 flt3::operator+(const flt3& a) const
		{
			return {a.x + x, a.y + y, a.z + z};
		}

		jmath::flt3 flt3::operator*(const float a) const
		{
			return {x * a, y * a, z * a};
		}

		vec1 toRadians(vec1 degree)
		{
			return DirectX::XMConvertToRadians(degree);
		}

		mat4x4 perspectiveFovLH_ZO(float fovRadians, float width, float height, float nearPlane, float farPlane)
		{
			return DirectX::XMMatrixPerspectiveFovLH(fovRadians, width / height, nearPlane, farPlane);
		}

		mat4x4 orthoLH_ZO(float left, float right, float bottom, float top, float nearPlane, float farPlane)
		{
			return DirectX::XMMatrixOrthographicOffCenterLH(left, right, bottom, top, nearPlane, farPlane);
		}

		mat4x4 lookAtLH(xvec4 eye, xvec4 focus, xvec4 up)
		{
			return DirectX::XMMatrixLookAtLH(eye, focus, up);
		}

		mat4x4 identity()
		{
			return DirectX::XMMatrixIdentity();
		}

		mat4x4 trs(const xvec4& translation, const quat& rotation, const xvec4& scale)
		{
			return DirectX::XMMatrixAffineTransformation(scale, xzero, rotation, translation);
		}

		xvec4 mul(const mat4x4& matrix, const xvec4& v)
		{
			return DirectX::XMVector4Transform(v, matrix);
		}

		xvec4 mul(const float f, const xvec4& v)
		{
			return DirectX::XMVectorScale(v, f);
		}

		mat4x4 mul(const mat4x4& matrix1, const mat4x4& matrix2)
		{
			return DirectX::XMMatrixMultiply(matrix1, matrix2);
		}

		quat mul(const quat& a, const quat& b)
		{
			return DirectX::XMQuaternionMultiply(a, b);
		}

		mat4x4 inverse(const mat4x4& val)
		{
			return DirectX::XMMatrixInverse(nullptr, val);
		}

		xvec4 loadPosition(const vec3& position)
		{
			const auto val = DirectX::XMLoadFloat3(&position);
			return DirectX::XMVectorSetW(val, 1);
		}

		xvec4 loadRotation(const vec3& rotation)
		{
			const auto val = DirectX::XMLoadFloat3(&rotation);
			return DirectX::XMVectorSetW(val, 0);
		}

		xvec4 loadVec4(const vec4& v)
		{
			return DirectX::XMLoadFloat4(&v);
		}

		vec3 toVec3(xvec4 v)
		{
			vec3 data;
			DirectX::XMStoreFloat3(&data, v);
			return data;
		}

		xvec4 rotate3(xvec4 v, quat q)
		{
			return DirectX::XMVector3Rotate(v, q);
		}

		quat eulerToQuat(const vec3& euler)
		{
			return DirectX::XMQuaternionRotationRollPitchYawFromVector(DirectX::XMLoadFloat3(&euler));
		}

		xvec4 angleAxis(const xvec4 axis, float angle)
		{
			return DirectX::XMQuaternionRotationAxis(axis, angle);
		}

		float min(float a, float b)
		{
			return a < b ? a : b;
		}

		vec3 min(const vec3& a, const vec3& b)
		{
			return vec3(min(a.x, b.x), min(a.y, b.y), min(a.z, b.z));;
		}

		float max(float a, float b)
		{
			return a > b ? a : b;
		}

		vec3 max(const vec3& a, const vec3& b)
		{
			return vec3(max(a.x, b.x), max(a.y, b.y), max(a.z, b.z));;
		}

		float sin(float radians)
		{
			return DirectX::XMScalarSin(radians);
		}

		float cos(float radians)
		{
			return DirectX::XMScalarCos(radians);
		}
	}

	jmath::xvec4 operator+(const jmath::xvec4& v1, const jmath::xvec4& v2)
	{
		return DirectX::XMVectorAdd(v1, v2);
	}
}
