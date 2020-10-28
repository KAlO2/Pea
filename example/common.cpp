#include "common.h"

#include <chrono>

#include "graphics/Image_PNG.h"


namespace pea {

void onWindowResize(GLFWwindow* window, int width, int height)
{
	(void)window;  // suppress unused parameter warning
	glViewport(0, 0, width, height);
}

int64_t getCurrentTime()
{
	using namespace std::chrono;
	return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

std::string formatCurrentTime()
{
	std::time_t t = std::time(nullptr);
	std::tm tm = *std::localtime(&t);
//	std::cout.imbue(std::locale("zh_CN.utf8"));

	std::ostringstream oss;
	oss << std::put_time(&tm, "%Y%m%d-%H%M%S");
	
	return oss.str();
}

void snapshot(const std::string& path, int32_t width, int32_t height)
{
	constexpr Color::Format format = Color::Format::C4_U8;
	Image_PNG image(width, height, format);
	uint8_t* pixels = image.getData();
	
	const int32_t rowStride = width * Color::size(format);
	int32_t alignment = 1;
	for(int32_t i = rowStride; (i & 1) == 0; i >>= 1)
	{
		alignment <<= 1;
		
		// The allowable alignment values are 1, 2, 4, 8
		if(alignment >= 8)
			break;
	}
	glPixelStorei(GL_PACK_ALIGNMENT, alignment);
	
	glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	image.flipVertical();
	image.save(path);
}

Texture createThermosTexture(uint32_t width)
{
	assert(width > 0);
	
	std::vector<vec4f> colorRamp(width);
	vec3f hsv(0.0, 1.0, 0.0);
	for(uint32_t i = 0; i < width; ++i)
	{
		float weight = static_cast<float>(i) / width;
		
		// Use gamma correction to even out the color bands:
		// 0 blue - cyan - green - yellow - red - 1
		// narrows yellow/cyan band, and widens red/green/blue.
		// Gamma 1.0 produces the original Blender 2.79 color ramp.
		hsv[0] = (2.0f / 3.0f) * (1.0f - weight);  // hue
//		hsv[1] = 1.0;  // saturation
#if 0
		hsv[2] = std::pow(0.5f + 0.5f * weight);
		vec3f color = hsv2rgb(hsv);
#else
		constexpr float gamma = 1.5f;
		hsv[2] = std::pow(0.5f + 0.5f * weight, gamma);
		vec3f color = hsv2rgb(hsv);

		for(int i = 0; i < 3; i++)
			color[i] = std::pow(color[i], 1.0f / gamma);
#endif
		colorRamp[i] = vec4f(color.r, color.g, color.b, 1.0f);
	}
#if 0
	const uint32_t height = 32;
	Image_PNG image(width, height, Color::Format::RGBA_8888);
	for(uint32_t i = 0; i < width; ++i)
	{
		uint32_t color = ColorF::getColor(colorRamp[i]);
		for(uint32_t j = 0; j < height; ++j)
			image.setPixel(i, j, color);
	}
	image.save("color_ramp.png");
#endif

	constexpr GLenum target = GL_TEXTURE_1D;
	Texture texture(target);
//	Texture::Parameter parameter(10);
	
	texture.bind();
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);  // RGBA
	
	GLint format = GL_RGBA;
	GLenum type = GL_FLOAT;
	glTexImage1D(target, 0/* level */, format, width, 0/* border */, format, type, colorRamp.data());
	glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	
//	glGenerateMipmap(texture.getName());
	glGenerateMipmap(target);
	return texture;
}

}  // namespace pea
