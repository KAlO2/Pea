#ifndef PEA_MATH_PERLIN_NOISE_H_
#define PEA_MATH_PERLIN_NOISE_H_

#include <cstdint>

namespace pea {

using std::uint8_t;

/**
 * Perlin noise
 * Improving Noise  Ken Perlin (2002)  https://mrl.nyu.edu/~perlin/paper445.pdf
 * State of the Art in Procedural Noise Functions. A. Lagae and al. (2010)
 * Given procedural noise n(x, y, z), a repeating animation in [0, t] can be achieved through
 * f(x, y, z, t) = ((t - z) * n(x, y, z) + z * n(x, y, t - z)) / t;
 * and for 2d noise: f(x, y, t) = ((t - y) * n(x, y) + y * n(x, t - y)) / t;
 * Seamlessly tiling noise in 2D plane, that's two orthogonal directions.
 * f(x, y) = ((w - x) * (h - y) * n(x, y) +
 *            (x  * (h - y) * n(x - w, y) +
 *            (w - x) * y * n(x, y -h) +
              (x * y * n(x - w, y - h)) / (w*h)
 * https://mzucker.github.io/html/perlin-noise-math-faq.html
 * see {@link http://flafla2.github.io/2014/08/09/perlinnoise.html }
 */
template <typename T>
class PerlinNoise
{
private:
	static T dot(uint8_t hash, const T& x);
	static T dot(uint8_t hash, const T& x, const T& y);
	
	/**
	 * the dot product of a unit sphere vector and vec3(x, y, z)
	 */
	static T dot(uint8_t hash, const T& x, const T& y, const T& z);

	static constexpr bool USE_STATIC_PERMUTATION = false;
	static constexpr bool USE_STATIC_VECTOR = false;
	
	static constexpr uint32_t tableLength = 256;
	
	uint8_t permutation[tableLength];
	
//	T gradients1[tableLength];  // two unit vector
	T gradients2[tableLength << 1];  // circle
	T gradients3[tableLength *  3];  // sphere
	
	const uint8_t& at(int32_t i) const;
public:
	PerlinNoise();
	PerlinNoise(uint32_t seed);
	
	static T lerp(const T& start, const T& end, const T& amount);
	
	/**
	 * @brief Fade function as defined by Ken Perlin. f(x) = 6*t^5 - 15*t^4 + 10*t^3
	 * This eases coordinate values so that they will ease towards integral values.
	 * This ends up smoothing the final output.
	 *
	 * @param x [0, 1]
	 * @return [0, 1]
	 */
	static T fade(T t);

	/**
	 * @group noise
	 * @{
	 *
	 *
	 * @return value in interval [-1.0, +1.0]
	 * <a href = "http://webstaff.itn.liu.se/~stegu/TNM022-2005/perlinnoiselinks/perlin-noise-math-faq.html">
	 * Zucker, M. The Perlin noise math FAQ</a> A deeper look into the mathematics of Perlin noise.
	 */
	T evaluate(const T& x);
	T evaluate(const T& x, const T& y);
	T evaluate(const T& x, const T& y, const T& z);
//	T evaluate(const T& x, const T& y, const T& z, const T& w);
	/**
	 * @}
	 */

	/**
	 * fractal noise is the simplest form of fractal functions summing up a few octaves
	 * of the noise value with an ever decreasing (0 < gain < 1) amplitude
	 */
//	T octaveNoise(vec3<T>& point, int32_t octave = 8, T lacunarity = 2.0, T gain = 0.5);
};

template <typename T>
inline const uint8_t& PerlinNoise<T>::at(int32_t i) const
{
	constexpr uint32_t mask = tableLength - 1;
	return permutation[i & mask];
}

}  // namespace pea

#include "PerlinNoise.inl"
#endif  // PEA_MATH_PERLIN_NOISE_H_
