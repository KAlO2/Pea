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
	constexpr Color::Format format = Color::Format::RGBA_8888;
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


}  // namespace pea
