#include "geometry/RayCast.h"

namespace pea {

// https://zhuanlan.zhihu.com/p/303168568
vec3f reflect(const vec3f& incident, const vec3f& normal)
{
	return incident - 2 * dot(incident, normal) * normal;
}

vec3f refract(const vec3f& incident, const vec3f& normal, const float& eta)
{
	float dotValue = dot(incident, normal);
	float k = 1 - eta * eta * (1 - dotValue * dotValue);
	if(k >= 0)
		return eta * incident - (eta * dotValue + std::sqrt(k)) * normal;
	
	return vec3f(0, 0, 0);
}

float fresnelReflectance(const vec3f& I, const vec3f& N, float eta)
{
	float cos_i = -dot(I, N);
	float cos2_t = 1.0 - eta * eta * (1.0 - cos_i * cos_i);
	if(cos2_t < 0.0)
		return 1.0;
	float cos_t = std::sqrt(cos2_t);
	
	float r1 = (eta * cos_i - cos_t) / (eta * cos_i + cos_t);
	float r2 = (cos_i - eta * cos_t) / (cos_i + eta * cos_t);
	return (r1 * r1 + r2 * r2) * 0.5;
}

float schlickReflectance(const vec3f& I, const vec3f& N, float eta)
{
	float f0 = (eta - 1.0) / (eta + 1.0);
	f0 *= f0;
	float cos_theta = -dot(I, N);
	if(eta > 1.0)
	{
		float cos2_t = 1.0 - eta * eta * (1.0 - cos_theta * cos_theta);
		if(cos2_t < 0.0)
			return 1.0;
		cos_theta = std::sqrt(cos2_t);
	}
	
	float x = 1 - cos_theta;
#if USE_POWER
	float x5 = pow(x, 5.0);
#else
	float x2 = x * x;
	float x5 = x2 * x2 * x;
#endif
	return f0 + (1.0 - f0) * x5;
}

vec3f getClosestPoint(const Ray& ray, const vec3f& point)
{
/*
	 first calculate parameter t, then clamp t to [0, 1]

	    |A.proj(B)|   |A|*cos(A,B)   |A|*|B|*cos(A,B)   dot(A,B)
	t = ----------- = ------------ = ---------------- = --------
	        |B|           |B|            |B|*|B|        B.norm()

	const vec3f v = point - pos;
	T t = dot(dir, v) / dir.norm();
	t = clamp(t, 0, 1);
	return pos + dir * t;
*/
	const vec3f& origin = ray.getOrigin();
	const vec3f& direction = ray.getDirection();
	const vec3f v = point - origin;
	float d = dot(direction, v);
	if(d < 0)
		return origin;

	d /= direction.length2();
	constexpr float ONE = 1.0F;
	float t = (d < ONE)? d: ONE;
	return ray.at(t);
}

bool castRay(const Ray& ray, const Sphere& sphere, HitInfo& hitInfo)
{
	// p: ray position, d: ray direction, c: sphere center, r: sphere radius
	// (p + d * t - c)^2 = r^2; d^2 = 1.
	// t^2 + 2*dot(p - c, d)*t + (p - c)^2 - r^2 = 0
	// t^2+ 2bt + c = 0; b = dot(p - c, d), c = (p - c)^2 - r^2.
	float radius = sphere.getRadius();
	vec3f v = ray.getOrigin() - sphere.getPosition();
	float b = dot(v, ray.getDirection());
	float c = v.length2() - radius * radius;

	float d = b * b - c;  // (2b)^2 - 4ac = 4b^2 - 4c = 4*(b^2 - c)
	if(d <= 0)  // delta <= 0
		return false;

	// t1 = (-2b - sqrt(4*(b^2 - c))) / 2 = -b - sqrt(b^2 - c)
	// t2 = (-2b + sqrt(4*(b^2 - c))) / 2 = -b + sqrt(b^2 - c)
	// fast fail if ray origin outside sphere and ray pointing away from sphere
	if(b > 0)  // t1 < t2 < 0
		return false;
	b = -b;
	d = std::sqrt(d);
	float t1 = b - d;
	float t2 = b + d;
	
	// outside sphere, 0 < t1 < t2
	// inside sphere, t1 < 0 < t2
	// on the spher t1 == t2
	hitInfo.inside = c < 0;  // inside sphere or on the sphere
	float t = hitInfo.inside? t2: t1; 
	
	hitInfo.coordinate = t;
	vec3f hitPoint = ray.at(t);
	hitInfo.normal = (hitPoint - sphere.getPosition()) / std::copysign(radius, c);
	return true;
}

bool castRay(const Ray& ray, const Plane& plane, HitInfo& hitInfo)
{
	// plane: ax + by + cz + d = 0, N = (a, b, c);  ray: P + V * t
	// a*(p.x + v.x*t) + b*(p.y + v.y*t) + c*(p.z + v.z*t) + d = 0
	// (a*v.x + b*v.y + c*v.z) * t + a * p.x + b * p.y + c * p.z + d = 0
	// dot(N, V) * t + dot(N, P) + d = 0
	const vec3f& N = plane.getNormal();
	const vec3f& V = ray.getDirection();
	float cos_theta = dot(N, V);
	if(std::abs(cos_theta) <= std::numeric_limits<float>::epsilon())
		return false;

	float f = plane.classify(ray.getOrigin());
	float t = -f / cos_theta;
	if(t < 0)  // opposite side
		return false;

	hitInfo.coordinate = t;
	hitInfo.inside = f > 0;
	if(f > 0)
		hitInfo.normal = plane.getNormal();
	else
		hitInfo.normal = -plane.getNormal();
	return true;
}

bool castRay(const Ray& ray, const Cylinder& cylinder, const vec3f& position, HitInfo& hitInfo)
{
	vec3f P = ray.getOrigin() - position;
	vec3f D = ray.getDirection();
	float r = cylinder.getRadius();
	float h = cylinder.getHeight() / 2;
	
	// (P.x + D.x * t)^2 + (P.y + D.y * t)^2 = r^2
	// (D.x^2 + D.y^2)t^2 + 2(D.x*P.x + D.y*P.y)t + (P.x^2 + P.y^2) - r^2 = 0
	float a = D.x * D.x + D.y * D.y;
	float b = D.x * P.x + D.y * P.y;
	float c = P.x * P.x + P.y * P.y - r * r;
	// at^2 + 2bt + c = 0
	// ray direction is perpendicular to Z axis, quadratic equation degenerates to linear equation
	if(std::abs(a) < std::numeric_limits<float>::epsilon())
	{
		if(c >= 0)  // around cylinder
			return false;
		
		// now abs(D.z) == 1
		if(D.z > 0)  // D.z == 1
		{
			if(P.z >= h)  // over the top cap
				return false;
			else if(P.z > -h)  // in the cylinder, hit the top cap
			{
				hitInfo.coordinate = -P.z + h;
				hitInfo.inside = true;
			}
			else  // under the bottom cap, hit the bottom cap
			{
				hitInfo.coordinate = -P.z - h;
				hitInfo.inside = false;
			}
		}
		else  // D.z == -1
		{
			if(P.z < -h)  // under the bottom cap
				return false;
			else if(P.z > -h)  // in the cylinder, hit the bottom cap
			{
				hitInfo.coordinate = P.z + h;
				hitInfo.inside = true;
			}
			else  // over the top cap, hit the top cap
			{
				hitInfo.coordinate = P.z - h;
				hitInfo.inside = false;
			}
		}
		
		hitInfo.normal = vec3f(D.x, D.y, -D.z);  // (0, 0, -1)
		return true;
	}
	
	float d = b * b - a * c;  // 4b^2 - 4ac = 4(b^2 - ac)
	if(d <= 0)
		return false;
	// t1 = (-2b - sqrt(4b^2 - 4ac)) / (2a) = (-b - sqrt(b^2 - ac)) / a
	// t2 = (-2b + sqrt(4b^2 - 4ac)) / (2a) = (-b + sqrt(b^2 - ac)) / a
	// inside  cylinder: t1 < 0 < t2, t = t2
	// outside cylinder: 0 < t1 < t2, t = t1
	b = -b;
	d = std::sqrt(d);
	float t1 = (b - d) / a;
	float t2 = (b + d) / a;
	if(t2 < 0)
		return false;
	float t = t1 >= 0? t1: t2;
	
	vec3f hitPoint = P + D * t;
	if(std::abs(hitPoint.z) > h)  // over or under the cap
		return false;
	hitInfo.coordinate = t;
	hitInfo.inside = c < 0;
	
	// hitPoint.x^2 + hitPoint.y^2 = r^2
	hitInfo.normal = vec3f(-hitPoint.x / r, -hitPoint.y / r, 0);
	return true;
}

// An Efficient and Robust Ray–Box Intersection Algorithm
// http://people.csail.mit.edu/amy/papers/box-jgt.pdf
bool castRay(const Ray& ray, const BoundingBox& box, HitInfo& hitInfo)
{
	const vec3f& P = ray.getOrigin();
	const vec3f& D = ray.getDirection();
	const vec3f& L = box.getLowerBound();
	const vec3f& U = box.getUpperBound();
	
	constexpr float EPS = std::numeric_limits<float>::epsilon();
	constexpr float INF = std::numeric_limits<float>::infinity();
	// (min, max) for X, Y, Z axis
	float t[6] = { -INF, +INF, -INF, +INF, -INF, +INF };
	uint8_t array[6] = {0, 1, 2, 3, 4, 5};
	// fast fail
	// max(t[0], t[1], t[2]) < min(t[3], t[4], t[5])
	auto _swap = [&t, &array](uint8_t i, uint8_t j)
	{
		std::swap(t[i], t[j]);
		std::swap(array[i], array[j]);
	};
	
	// P.x + D.x * t0 = L.x;  P.x + D.x * t1 = U.x;
	float inv;
	if(std::abs(D.x) > EPS)
	{
		inv = 1 / D.x;
		t[0] = (L.x - P.x) * inv;
		t[1] = (U.x - P.x) * inv;
	}
	
	if(D.x < 0)  // or if(t_min.x > t_max.x)
		_swap(0, 1);
	
	if(std::abs(D.y) > EPS)
	{
		inv = 1 / D.y;
		t[2] = (L.y - P.y) * inv;
		t[3] = (U.y - P.y) * inv;
	}
	
	if(D.y < 0)  // or if(t_min.y > t_max.y)
		_swap(2, 3);
	
	if(t[0] > t[3] || t[1] < t[2])
		return false;
	
	if(t[2] > t[0])  // std::max
		_swap(0, 2);
	
	if(t[1] > t[3])  // std::min
		_swap(1, 3);
	
	if(std::abs(D.z) > EPS)
	{
		inv = 1 / D.z;
		t[4] = (L.z - P.z) * inv;
		t[5] = (U.z - P.z) * inv;
	}
	
	if(D.z < 0)  // or if(t_min.z > t_max.z)
		_swap(4, 5);
	
	if(t[0] > t[5] || t[1] < t[4])
		return false;
	
	if(t[4] > t[0])  // std::max
		_swap(0, 4);
	if(t[1] > t[5])  // std::min
		_swap(1, 5);
	
	// t_min < 0 < t_max, in the box
	// 0 < t_min < t_max, out of the box
	if(t[1] <= 0)
		return false;
	
	bool inBox = t[0] < 0;
	hitInfo.inside = inBox;
	hitInfo.coordinate = inBox? t[1]: t[0];
	const uint8_t& index = inBox? array[1]: array[0];
	vec3f normal(0, 0, 0);
#if 1
	normal[index / 2] = inBox == ((index & 1) == 0)? +1: -1;
#else
	switch(index)
	{
	case 0:  normal.x = inBox? +1: -1;  break;
	case 1:  normal.x = inBox? -1: +1;  break;
	case 2:  normal.y = inBox? +1: -1;  break;
	case 3:  normal.y = inBox? -1: +1;  break;
	case 4:  normal.z = inBox? +1: -1;  break;
	case 5:  normal.z = inBox? -1: +1;  break;
	default: assert(false);
	}
#endif
	hitInfo.normal = normal;
	return true;
}

bool castRay(const Ray& ray, const vec3f vertices[3], HitInfo& hitInfo)
{
	vec3f p01 = vertices[1] - vertices[0];
	vec3f p02 = vertices[2] - vertices[0];
	vec3f pvec = cross(ray.getDirection(), p02);
	float det = dot(pvec, p01);

	// ray and triangle are parallel if det is close to 0
	if(std::abs(det) < std::numeric_limits<float>::epsilon())
		return false;

	float invDet = 1 / det;

	vec3f tvec = ray.getOrigin() - vertices[0];
	float u = dot(tvec, pvec) * invDet;
	if(u < 0 || u > 1)
		return false;

	vec3f qvec = cross(tvec, p01);
	float v = dot(ray.getDirection(), qvec) * invDet;
	if(v < 0 || u + v > 1)
		return false;

	float test = dot(p02, qvec) * invDet;
	return test > 0;
}

}  // namespace pea
