#include "math/Transform.h"

#include <stdexcept>

using namespace pea;

Transform::Transform():
		translation(0, 0, 0),
		rotation(0, 0, 0),
		scaling(1, 1, 1),
		axisLock(0)
{
}

void Transform::setTransform(const mat4f& transform)
{
	const mat4f& m = transform;
	translation = vec3f(m[0][3], m[1][3], m[2][3]);
	scaling.x = vec3f(m[0][0], m[1][0], m[2][0]).length();
	scaling.y = vec3f(m[0][1], m[1][1], m[2][2]).length();
	scaling.z = vec3f(m[0][2], m[1][2], m[2][2]).length();
	
//	float r00 = cy * cz, r01 = sx * sy * cz - cx * sz, r02 = cx * sy * cz + sx * sz;
//	float r10 = cy * sz, r11 = sx * sy * sz + cx * cz, r12 = cx * sz * sz - sx * cz;
//	float r20 =-sy,      r21 = sx * cy,                r22 = cx * cy;
	// m[2][0] = -sin(scale.y);
	float r20 = m[2][0] / scaling.x;
	if(std::abs(r20) < 1)
	{
		scaling.y = -std::asin(r20);  // ry1, ry2 = pi - ry;
		// m[2][1] / m[2][2] = tan(scale.x);
		float cy = std::cos(scaling.y);
		scaling.x = std::atan2(m[2][1] / cy, m[2][2] / cy);
		
		// m[1][0] / m[0][0] = tan(scale.z)
		scaling.z = std::atan2(m[1][0] / cy, m[0][0] / cy);
	}
	else  // Rz(phi), Ry(theta), Rx(psai)
	{
		// rotation.phi = anything; can set to 0
		rotation.z = 0;
		if(r20 < 0)  // r20 == -1
		{
			rotation.y = M_PI / 2;
			rotation.x = rotation.z + std::atan2(m[0][1], m[0][2]);
		}
		else  // R20 == +1
		{
			rotation.y = -M_PI / 2;
			rotation.x = -rotation.z - std::atan2(m[0][1], m[0][2]);
		}
	}
}

mat4f Transform::getTransform() const
{
/*
There are six possible ways you can describe rotation using Euler angles — X-Y-Z, X-Z-Y, Y-Z-X, Y-X-Z, Z-X-Y, Z-Y-X. Now you are thinking, the choice is easy. Let’s just choose X-Y-Z. Right ? Wrong. The industry standard is Z-Y-X because that corresponds to yaw, pitch and roll.
*/
	// https://www.gregslabaugh.net/publications/euler.pdf
	// R = Rz * Ry * Rx
	float cx = std::cos(rotation.x), sx = std::sin(rotation.x);
	float cy = std::cos(rotation.y), sy = std::sin(rotation.y);
	float cz = std::cos(rotation.z), sz = std::sin(rotation.z);
	float r00 = cy * cz, r01 = sx * sy * cz - cx * sz, r02 = cx * sy * cz + sx * sz;
	float r10 = cy * sz, r11 = sx * sy * sz + cx * cz, r12 = cx * sz * sz - sx * cz;
	float r20 =-sy,      r21 = sx * cy,                r22 = cx * cy;
	
	mat4f m;  // T * R * S
	m[0][0] = r00 * scaling.x;  m[0][1] = r01 * scaling.y;  m[0][2] = r02 * scaling.z;  m[0][3] = translation.x;
	m[1][0] = r10 * scaling.x;  m[1][1] = r11 * scaling.y;  m[1][2] = r12 * scaling.z;  m[1][3] = translation.y;
	m[2][0] = r20 * scaling.x;  m[2][1] = r21 * scaling.y;  m[2][2] = r22 * scaling.z;  m[2][3] = translation.z;
	m[3][0] =             0;  m[3][1] =             0;  m[3][2] =             0;  m[3][3] =             1;
	return m;
}

void Transform::setTransform(uint8_t axis, const vec3f& variable)
{
	switch(axis)
	{
	case Transform::T:  translation = variable;  break;
	case Transform::R:  rotation    = variable;  break;
	case Transform::S:  scaling     = variable;  break;
	default:  throw std::invalid_argument("axis must be Transform::T/R/S");
	}
}

const vec3f& Transform::getTransform(uint8_t axis) const
{
	switch(axis)
	{
	case Transform::T:  return translation;
	case Transform::R:  return rotation;
	case Transform::S:  return scaling;
	default:  throw std::invalid_argument("axis must be Transform::T/R/S");
	}
}

void Transform::translate(uint8_t axis, float offset)
{
	axis &= ~static_cast<uint8_t>(axisLock >> TRANSLATION_SHIFT);
	if((axis & X) != 0)
		translation.x += offset;
	if((axis & Y) != 0)
		translation.y += offset;
	if((axis & Z) != 0)
		translation.z += offset;
}

void Transform::rotate(uint8_t axis, float angle)
{
	axis &= ~static_cast<uint8_t>(axisLock >> ROTATION_SHIFT);
	if((axis & X) != 0)
		rotation.x += angle;
	if((axis & Y) != 0)
		rotation.y += angle;
	if((axis & Z) != 0)
		rotation.z += angle;
}

void Transform::scale(uint8_t axis, float factor)
{
	axis &= ~static_cast<uint8_t>(axisLock >> SCALING_SHIFT);
	if((axis & X) != 0)
		scaling.x *= factor;
	if((axis & Y) != 0)
		scaling.y *= factor;
	if((axis & Z) != 0)
		scaling.z *= factor;
}

void Transform::reset()
{
	translation = vec3f(0, 0, 0);
	rotation = vec3f(0, 0, 0);
	scaling = vec3f(1, 1, 1);
}
