#include "math/PerlinNoise.h"
#include "math/Random.h"
#include "graphics/Image_PNG.h"

using namespace pea;

static constexpr uint8_t map(const float& x)
{
	// [-1.0, +1.0] -> [0, 255]  // (x + 1) * 0.5F * 255
	// std::abs(x * 255)
	return static_cast<uint8_t>((x + 1) * 0.5F * 255);
}

static void fillImage(Image& image, uint32_t (*function)(uint32_t i, uint32_t j))
{
	const uint32_t width = image.getWidth();
	const uint32_t height= image.getHeight();
	for(uint32_t j = 0; j < height; ++j)
	for(uint32_t i = 0; i < width; ++i)
	{
		uint32_t color = function(i, j);
		image.setPixel(i, j, color);
	}
}

static float random(float x, float y)
{
	float value = std::sin(12.9898 * x + 78.233 * y) * 43758.5453123;
	float fractional;
	std::modf(value, &fractional);
	return fractional;
}
/*
static float noise(float x, float y)
{
	float x_f = std::floor(x), y_f = std::floor(y);
	// std::modf return negtive value for negative value.
	float x_i = std::modf(x, &x_f);
	float y_i = std::modf(y, &y_f);
	
	return 0;
}
*/

// https://lodev.org/cgtutor/randomnoise.html
int main()
{
	PerlinNoise<float> noise;
	
	uint32_t width = 512, height = 512;
	Image_PNG image(width, height, Color::Format::RGB_888);
	constexpr float frequency = 16.0F;
	
	auto random = [](uint32_t i, uint32_t j)
	{
		std::ignore = i;
		std::ignore = j;
		
		float value = Random::emit();
		assert(0 <= value && value < 1.0);
		uint8_t r = static_cast<uint8_t>(value * 256);
#if 1
		value = Random::emit();
		assert(0 <= value && value < 1.0);
		uint8_t g = static_cast<uint8_t>(value * 256);
		Random::emit();Random::emit();Random::emit();
		value = Random::emit();
		assert(0 <= value && value < 1.0);
		uint8_t b = static_cast<uint8_t>(value * 256);
		
		return Color::from_RGB888(r, g, b);
#else
		return Color::from_G8(gray);
#endif
	};

	fillImage(image, random);
	image.save("random.png");
	
//	auto noise1d = [&noise](uint32_t i, uint32_t /* j */) { return Color::from_G8(map(noise.evaluate(i * 31.0F))); };
//	fillImage(image, noise1d);
//	image.save("noise1d.png");
	
	for(uint32_t j = 0; j < height; ++j)
	for(uint32_t i = 0; i < width; ++i)
	{
		float value = noise.evaluate(i * frequency, j * frequency);  // [-1.0, +1.0]
		uint8_t gray = map(value);
		image.setPixel(i, j, Color::from_G8(gray));
	}
	image.save("noise2d.png");
	
	for(uint32_t j = 0; j < height; ++j)
	for(uint32_t i = 0; i < width; ++i)
	{
		float x = i * frequency, y = j * frequency, z = std::sin(x) * y;
		float value = noise.evaluate(x, y, z);  // [-1.0, +1.0]
		uint8_t gray = map(value);
		image.setPixel(i, j, Color::from_G8(gray));
	}
	// TODO: display GIF
	image.save("noise3d_cutaway.png");
	
	for(uint32_t j = 0; j < height; ++j)
	for(uint32_t i = 0; i < width; ++i)
	{
		float x = i * frequency, y = j * frequency;
		float value = noise.evaluate(x, y); 
		value *= 10;
		value -= static_cast<int>(value);
		uint8_t gray = map(value);
		image.setPixel(i, j, Color::from_G8(gray));
	}
	image.save("wood.png");
	
	constexpr uint32_t layerCount = 5;
	float frequencyMultiplier = 2.0; 
	float amplitudeMultiplier = 0.50;
	float currentFrequency = frequency;
	for(uint32_t j = 0; j < height; ++j)
	for(uint32_t i = 0; i < width; ++i)
	{
		float value = 0;
		float amplitude = 1.0F;
		for(uint32_t l = 0; l < layerCount; ++l)
		{
			value += amplitude * noise.evaluate(i * currentFrequency, j * currentFrequency);
			currentFrequency /= frequencyMultiplier;
			amplitude *= amplitudeMultiplier;
		}
		
		// we "displace" the value i used in the sin() expression by noiseValue * 100
		uint8_t gray = (std::sin((i + value * 100) * 2 * M_PI / 200.f) + 1) / 2.f * 255;
		image.setPixel(i, j, Color::from_G8(gray));
	}
	image.save("turbulant_noise.png");
	
	currentFrequency = frequency;
	for(uint32_t j = 0; j < height; ++j)
	for(uint32_t i = 0; i < width; ++i)
	{
		float value = 0;
		float amplitude = 1.0F;
		for(uint32_t l = 0; l < 3; ++l)
		{
			value += amplitude * noise.evaluate(i * currentFrequency, j * currentFrequency);
			currentFrequency *= 2.0;
			amplitude *= 0.35;
		}
		
		value = std::sin(i * frequency + value);
		// we "displace" the value i used in the sin() expression by noiseValue * 100
		uint8_t gray = (std::sin((i + value * 100) * 2 * M_PI / 200.f) + 1) / 2.f * 255;
		image.setPixel(i, j, Color::from_G8(gray));
	}
	image.save("marble.png");
	
	return 0;
}
