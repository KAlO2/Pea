#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <random>


// putting #include on the first line makes gedit syntax highlight.
#ifndef PEA_MATH_PERLIN_NOISE_H_
#error "please #include \"math/PerlinNoise.h\""
#endif

namespace pea {

template <typename T>
PerlinNoise<T>::PerlinNoise():
		PerlinNoise(std::random_device{}())
{
}

template <typename T>
PerlinNoise<T>::PerlinNoise(const uint32_t seed)
{
	std::mt19937 generator(seed);
	std::uniform_real_distribution<T> distribution;
	auto random = [&distribution, &generator]() { return distribution(generator); };  // [0.0, 1.0)
	
	for(uint32_t i = 0; i < tableLength; ++i)
	{
#if 0  // not efficient approach
		constexpr T LOW = 1E-8;  // In case vector is divided by zero, make length > 0.
		while(true)
		{
			T x = random(), y = random();
			T length2 = x * x + y * y;
			if(LOW < length2 && length2 < 1)
			{
				T length = std::sqrt(length2);
				gradients2[i * 2    ] = x / length;
				gradients2[i * 2 + 1] = y / length;
				break;
			}
		}
		
		while(true)
		{
			T x = random(), y = random(), z = random();
			T length2 = x * x + y * y + z * z;
			if(LOW < length2 && length2 < 1)
			{
				T length = std::sqrt(length2);
				gradients3[i * 3    ] = x / length;
				gradients3[i * 3 + 1] = y / length;
				gradients3[i * 3 + 2] = z / length;
				break;
			}
		}
#else
		T theta = 2 * M_PI * random();
		T phi   = std::acos(2 * random() - 1);
		
		T cos_phi = std::cos(phi), cos_theta = std::cos(theta);
		T sin_phi = std::sin(phi), sin_theta = std::sin(theta);
		
		gradients2[i * 2    ] = cos_theta;
		gradients2[i * 2 + 1] = sin_theta;
		
		gradients3[i * 3    ] = cos_phi * cos_theta;
		gradients3[i * 3 + 1] = cos_phi * sin_theta;
		gradients3[i * 3 + 2] = sin_phi;
#endif
	}
	
	if constexpr(USE_STATIC_PERMUTATION)
	{
		// Hash lookup table as defined by Ken Perlin.
		const uint8_t PERMUTATION[tableLength] =
		{
			// arranged array of all numbers from 0 to 255, both inclusive.
			151, 160, 137,  91,  90,  15, 131,  13, 201,  95,  96,  53, 194, 233,   7, 225,
			140,  36, 103,  30,  69, 142,   8,  99,  37, 240,  21,  10,  23, 190,   6, 148,
			247, 120, 234,  75,   0,  26, 197,  62,  94, 252, 219, 203, 117,  35,  11,  32,
			 57, 177,  33,  88, 237, 149,  56,  87, 174,  20, 125, 136, 171, 168,  68, 175,
			 74, 165,  71, 134, 139,  48,  27, 166,  77, 146, 158, 231,  83, 111, 229, 122,
			 60, 211, 133, 230, 220, 105,  92,  41,  55,  46, 245,  40, 244, 102, 143,  54,
			 65,  25,  63, 161,   1, 216,  80,  73, 209,  76, 132, 187, 208,  89,  18, 169,
			200, 196, 135, 130, 116, 188, 159,  86, 164, 100, 109, 198, 173, 186,   3,  64,
			 52, 217, 226, 250, 124, 123,   5, 202,  38, 147, 118, 126, 255,  82,  85, 212,
			207, 206,  59, 227,  47,  16,  58,  17, 182, 189,  28,  42, 223, 183, 170, 213,
			119, 248, 152,   2,  44, 154, 163,  70, 221, 153, 101, 155, 167,  43, 172,   9,
			129,  22,  39, 253,  19,  98, 108, 110,  79, 113, 224, 232, 178, 185, 112, 104,
			218, 246,  97, 228, 251,  34, 242, 193, 238, 210, 144,  12, 191, 179, 162, 241,
			 81,  51, 145, 235, 249,  14, 239, 107,  49, 192, 214,  31, 181, 199, 106, 157,
			184,  84, 204, 176, 115, 121,  50,  45, 127,   4, 150, 254, 138, 236, 205,  93,
			222, 114,  67,  29,  24,  72, 243, 141, 128, 195,  78,  66, 215,  61, 156, 180,
		};
		
		std::copy(PERMUTATION, PERMUTATION + tableLength, permutation);
	}
	else
	{
		// fill permutation with values from 0 (inclusive) to tableLength (exclusive).
		std::iota(permutation, permutation + tableLength, 0);
		// uniform_real_distribution's interval is [a, b), while uniform_int_distribution's interval
		// is [a, b], create permutation table [0, 256)
		std::shuffle(permutation, permutation + tableLength, generator);
	}
}

template <typename T>
T PerlinNoise<T>::dot(uint8_t hash, const T& x)
{
	return (hash & 0x1) == 0? x: -x;
}

template <typename T>
T PerlinNoise<T>::dot(uint8_t hash, const T& x, const T& y)
{
	switch(hash & 0x3)
	{
	case 0x0: return  x + y;  // ( 1, 1)
	case 0x1: return -x + y;  // (-1, 1)
	case 0x2: return  x - y;  // ( 1,-1)
	case 0x3: return -x - y;  // (-1,-1)
	}
	
	throw std::runtime_error("unreachable code");
}

template <typename T>
T PerlinNoise<T>::dot(uint8_t hash, const T& x, const T& y, const T& z)
{
	switch(hash & 0xF)  // low 4 bits
	{
	// To avoid the cost of dividing by 12, we pad to 16 gradient directions, adding an extra 
	// (1,1,0), (-1,1,0), (0,-1,1) and (0,-1,-1). These form a regular tetrahedron, so adding them
	// redundantly introduces no visual bias in the texture.
	case 0xC:
	case 0x0: return  x + y;  // ( 1, 1, 0)
	case 0xD:
	case 0x1: return -x + y;  // (-1, 1, 0)
	case 0x2: return  x - y;  // ( 1,-1, 0)
	case 0x3: return -x - y;  // (-1,-1, 0)
	case 0x4: return  x + z;  // ( 1, 0, 1)
	case 0x5: return -x + z;  // (-1, 0, 1)
	case 0x6: return  x - z;  // ( 1, 0,-1)
	case 0x7: return -x - z;  // (-1, 0,-1)
	case 0x8: return  y + z;  // ( 0, 1, 1)
	case 0xE:
	case 0x9: return -y + z;  // ( 0,-1, 1)
	case 0xA: return  y - z;  // ( 0, 1,-1)
	case 0xF:
	case 0xB: return -y - z;  // ( 0,-1,-1)
	}
	
	throw std::runtime_error("unreachable code");
}

template <typename T>
T PerlinNoise<T>::lerp(const T& start, const T& end, const T& amount)
{
	return start + (end - start) * amount;
}

template <typename T>
T PerlinNoise<T>::fade(T t)
{
	// f(t) = 3 * t^2 - 2 * t^3;
	// second derivative: f''(t) = 6 - 12 * t; which is not zero at either t=0 or t=1.
	// This non-zero value creates second order discontinuities across the coordinate-aligned faces
	// of adjoining cubic cells. 
//	return t * t * (2 - 3 * t);

	// f(t) = 6 * t^5 - 15 * t^4 + 10 * t^3
	// econd derivative: f''(t) = 120 * t^3 - 180 * t^2 + 60 * t = 60 * (1 - 2*t) * (1 - t) * t;
	return t * t * t * (t * (t * 6 - 15) + 10);
}

template <typename T>
T PerlinNoise<T>::evaluate(const T& x)
{
	// unit grid cell containing point
	T x_ = std::floor(x);
	int32_t i = static_cast<int32_t>(x_);
	
	// relative X coordinates of point within that cell
	T w0 = x - x_;
	T w1 = w0 - 1;
	T u = fade(w0);

	// calculate two hashed gradient
	const T gradients[2] = {1, -1};
	const T& g0 = gradients[at(i)];
	const T& g1 = gradients[at(i + 1)];

	// interpolate along x direction.
	T nx = lerp(g0 * w0, g1 * w1, u);

	return nx;
}

template <typename T>
T PerlinNoise<T>::evaluate(const T& x, const T& y)
{
	// unit grid cell containing point
	T x_ = std::floor(x), y_ = std::floor(y);
	int32_t i = static_cast<int32_t>(x_);
	int32_t j = static_cast<int32_t>(y_);
	
	// relative XY coordinates of point within that cell
	T x0 = x - x_, y0 = y - y_;
	T x1 = x0 - 1, y1 = y0 - 1;
	T u = fade(x0), v = fade(y0);

	// calculate a set of eight hashed gradient
	// leave X, Y without brackets intentionally, you will see.
	auto hash = [this](int32_t i, int32_t j) { return at(at(i) + j); };
	const uint8_t& h00 = hash(i, j);
	const uint8_t& h01 = hash(i, j + 1);
	const uint8_t& h10 = hash(i + 1, j);
	const uint8_t& h11 = hash(i + 1, j + 1);
	
	T p00, p01, p10, p11;
	if constexpr(USE_STATIC_VECTOR)
	{
		p00 = dot(h00, x0, y0);
		p01 = dot(h01, x0, y1);
		p10 = dot(h10, x1, y0);
		p11 = dot(h11, x1, y1);
	}
	else
	{
		const T* g00 = gradients2 + h00 * 2;
		const T* g01 = gradients2 + h01 * 2;
		const T* g10 = gradients2 + h10 * 2;
		const T* g11 = gradients2 + h11 * 2;
	
		// dot product
		p00 = g00[0] * x0 + g00[1] * y0;
		p01 = g01[0] * x0 + g01[1] * y1;
		p10 = g10[0] * x1 + g10[1] * y0;
		p11 = g11[0] * x1 + g11[1] * y1;
	}
	
	// interpolate along X/Y directions separately.
	T nx0 = lerp(p00, p10, u);
	T nx1 = lerp(p01, p11, u);

	T nxy = lerp(nx0, nx1, v);
	return nxy;
}

template <typename T>
T PerlinNoise<T>::evaluate(const T& x, const T& y, const T& z)
{
	// unit grid cell containing point
	T x_ = std::floor(x), y_ = std::floor(y), z_ = std::floor(z);
	int32_t i = static_cast<int32_t>(x_);
	int32_t j = static_cast<int32_t>(y_);
	int32_t k = static_cast<int32_t>(z_);
	
	// relative XY coordinates of point within that cell
	T x0 = x - x_, y0 = y - y_, z0 = z - z_;
	T x1 = x0 - 1, y1 = y0 - 1, z1 = z0 - 1;
	T u = fade(x0), v = fade(y0), w = fade(z0);

	// calculate a set of eight hashed gradient
	// leave X, Y, Z without brackets intentionally, you will see.
	auto hash = [this](int32_t i, int32_t j, int32_t k) { return at(at(at(i) + j) + k); };
	const uint8_t& h000 = hash(i, j, k);
	const uint8_t& h001 = hash(i, j, k + 1);
	const uint8_t& h010 = hash(i, j + 1, k);
	const uint8_t& h011 = hash(i, j + 1, k + 1);
	
	const uint8_t& h100 = hash(i + 1, j, k);
	const uint8_t& h101 = hash(i + 1, j, k + 1);
	const uint8_t& h110 = hash(i + 1, j + 1, k);
	const uint8_t& h111 = hash(i + 1, j + 1, k + 1);
	
	T p000, p001, p010, p011, p100, p101, p110, p111;
	if constexpr(USE_STATIC_VECTOR)
	{
		p000 = dot(h000, x0, y0, z0);
		p001 = dot(h001, x0, y0, z1);
		p010 = dot(h010, x0, y1, z0);
		p011 = dot(h011, x0, y1, z1);
		
		p100 = dot(h100, x1, y0, z0);
		p101 = dot(h101, x1, y0, z1);
		p110 = dot(h110, x1, y1, z0);
		p111 = dot(h111, x1, y1, z1);
	}
	else
	{
		const T* g000 = gradients3 + h000 * 3;
		const T* g001 = gradients3 + h001 * 3;
		const T* g010 = gradients3 + h010 * 3;
		const T* g011 = gradients3 + h011 * 3;
		const T* g100 = gradients3 + h100 * 3;
		const T* g101 = gradients3 + h101 * 3;
		const T* g110 = gradients3 + h110 * 3;
		const T* g111 = gradients3 + h111 * 3;
	
		// dot product
		p000 = g000[0] * x0 + g000[1] * y0 + g000[2] * z0;
		p001 = g001[0] * x0 + g001[1] * y0 + g001[2] * z1;
		p010 = g010[0] * x0 + g010[1] * y1 + g010[2] * z0;
		p011 = g011[0] * x0 + g011[1] * y1 + g011[2] * z1;
		
		p100 = g100[0] * x1 + g100[1] * y0 + g100[2] * z0;
		p101 = g101[0] * x1 + g101[1] * y0 + g101[2] * z1;
		p110 = g110[0] * x1 + g110[1] * y1 + g110[2] * z0;
		p111 = g111[0] * x1 + g111[1] * y1 + g111[2] * z1;
	}
	
	// interpolate along X/Y/Z directions separately.
	T nx00 = lerp(p000, p100, u);
	T nx01 = lerp(p001, p101, u);
	T nx10 = lerp(p010, p110, u);
	T nx11 = lerp(p011, p111, u);

	T nxy0 = lerp(nx00, nx10, v);
	T nxy1 = lerp(nx01, nx11, v);

	T nxyz = lerp(nxy0, nxy1, w);

	return nxyz;
}

/**
 * http://en.wikipedia.org/wiki/Fractional_Brownian_motion
 */
#if 0
template <typename T>
T PerlinNoise<T>::octaveNoise(vec3<T>& point, int32_t octave/* = 8*/, T lacunarity/* = 2.0*/, T gain/* = 0.5*/)
{
	T amplitude = 1.0, frequency = 1.0;
	T sum = 0.0, max = 0.0;

	// Noise basis functions are generally designed to return value in range [-1, 1],
	// so the sum can goes out of this range. Divided by max can solve the problem.
	for(int32_t i = 0; i < octave; ++i)
	{
		sum += amplitude * noise(frequency * point);
		max += amplitude;

		amplitude *= gain;
		frequency *= lacunarity;
	}

	return sum / max;
}
#endif
}  // namespace pea
