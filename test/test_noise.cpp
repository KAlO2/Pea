#include "math/PerlinNoise.h"
#include "graphics/Image_PNG.h"

using namespace pea;

static constexpr uint8_t map(const float& x)
{
	// [-1.0, +1.0] -> [0, 255]  // (x + 1) * 0.5F * 255
	// std::abs(x * 255)
	return static_cast<uint8_t>((x + 1) * 0.5F * 255);
}

int main()
{
	PerlinNoise<float> noise;
	
	uint32_t width = 1024, height = 1024;
	Image_PNG image(width, height, Color::Format::G_8);
	constexpr float frequency = 1 / 64.0F;
	std::vector<uint8_t> array(width);
	for(uint32_t i = 0; i < width; ++i)
	{
		float value = noise.evaluate(i * 31.0F);
		array[i] = map(value);
	}
	for(uint32_t j = 1; j < height; ++j)
	for(uint32_t i = 0; i < width; ++i)
		image.setPixel(i, j, Color::from_G8(array[i]));
	image.save("noise1d.png");
	
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
