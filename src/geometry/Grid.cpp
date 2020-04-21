#include "geometry/Grid.h"

#include <sstream>

using namespace pea;


/**
 * vector = [start:step:stop];       // MATLAB syntax
 * vector = range(start, stop, step)  # Python syntax, but it doesn't support the float type.
 *
 * @param[in] start Starting number of the sequence.
 * @param[in] stop Generate numbers up to, this number is included too.
 * @param[in] steps At least 1 step from start to stop.
 * @return length of steps + 1 number sequence
 */
template <typename T>
static std::vector<T> sequence(const T& start, const T& stop, const size_t& steps)
{
	assert(steps > 0);
	const T step = (stop - start) / steps;

	std::vector<T> v(steps + 1);
	for(size_t i = 0; i < steps; ++i)
		v[i] = start + i * step;
	v[steps] = stop;
	
	return v;
}

std::vector<vec3f> Grid::getVertexData(const float& start, const float& stop, const uint32_t& steps)
{
	std::vector<float> v = sequence<float>(start, stop, steps);
	const uint32_t N = steps + 1;  // v.size();
	
	std::vector<vec3f> vertices;
	vertices.reserve(N * N);
	for(uint32_t r = 0; r < N; ++r)
		for(uint32_t c = 0; c < N; ++c)
			vertices.push_back(vec3f(v[c], v[r], 0));

	return vertices;
}

std::vector<vec3f> Grid::getVertexData(const vec2f& start, const vec2f& stop, const vec2u& steps)
{
	std::vector<float> vx = sequence<float>(start.x, stop.x, steps.x);
	std::vector<float> vy = sequence<float>(start.y, stop.y, steps.y);
	const uint32_t M = steps.x + 1, N = steps.y + 1; 

	std::vector<vec3f> vertices;
	vertices.reserve(M * N);
	for(uint32_t r = 0; r < N; ++r)
		for(uint32_t c = 0; c < M; ++c)
			vertices.emplace_back(vx[c], vy[r], 0.0F);

	return vertices;
}

std::vector<vec3f> Grid::getVertexData(const uint32_t& stepsX, const uint32_t& stepsY, const float& step)
{
	assert(stepsX > 0 && stepsY > 0);
	
	std::vector<float> X(stepsX + 1), Y(stepsY + 1);
	float centerX = stepsX * (step / 2);
	float centerY = stepsY * (step / 2);
	for(uint32_t i = 0; i <= stepsX; ++i)
		X[i] = i * step - centerX;
	for(uint32_t j = 0; j <= stepsY; ++j)
		Y[j] = j * step - centerY;
	
	const uint32_t size = getVertexSize(stepsX, stepsY);
#if 0
	std::vector<vec3f> vertices(size);
	for(uint32_t i = 0; i < size; ++i)
	{
		int32_t r = i / (stepsX + 1);
		int32_t c = i % (stepsX + 1);
		vertices[i] = vec3f(X[c], Y[r], 0.0F);
	}
#else
	std::vector<vec3f> vertices;
	vertices.reserve(size);
	for(uint32_t j = 0; j <= stepsY; ++j)
		for(uint32_t i = 0; i <= stepsX; ++i)
			vertices.emplace_back(X[i], Y[j], 0.0F);
//	assert(vertices.size() == static_cast<size_t>(size));
#endif
	
	return vertices;
}

std::vector<vec2f> Grid::getTexcoordData(const float& width, const float& height, const uint32_t& stepsX, const uint32_t& stepsY)
{
	std::vector<float> tx = sequence<float>(0.0F, width, stepsX);
	std::vector<float> ty = sequence<float>(0.0F, height, stepsY);

	std::vector<vec2f> texcoords;
	texcoords.reserve(getVertexSize(stepsX, stepsY));
	for(uint32_t j = 0; j <= stepsY; ++j)
		for(uint32_t i = 0; i <= stepsX; ++i)
			texcoords.emplace_back(tx[i], ty[j]);

	return texcoords;
}

std::vector<uint32_t> Grid::getIndexData(const uint32_t& stepsX, const uint32_t& stepsY, bool strip/* = true*/)
{
	assert(stepsX > 0 && stepsY > 0);
	
	// glDrawElements use type GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, or GL_UNSIGNED_INT.
	// note that `typename` is needed here, or C++ standard assumes std::remove_cv<T>::type is a
	// variable rather than a type.
#if 0
	using U = typename std::remove_cv<T>::type;
	static_assert(std::is_same<U, uint8_t>::value
			|| std::is_same<U, uint16_t>::value
			|| std::is_same<U, uint32_t>::value,
			"types other than uint8_t/uint16_t/uint32_t are not allowed");
#else
	using T = uint32_t;
#endif

	const uint32_t count = getIndexSize(stepsX, stepsY);
	std::vector<T> indices;
	indices.reserve(count);

	const uint32_t incrementX = stepsX + 1;
	if(strip)
	{
		for(uint32_t j = 1; j <= stepsY; ++j)
		{
			T offset = j * incrementX;
			for(uint32_t i = 0; i <= stepsX; ++i)
			{
				indices.push_back(offset + i);
				indices.push_back(offset + i - incrementX);
			}
			
			// extra two form two degenerated triangles
			// append each row with 2 extra indices (to form degenerated triangles), except the last row.
			if(j != stepsY)
			{
				indices.push_back(offset - 1);  // same as previous index
				indices.push_back(offset + incrementX);  // same as next index
			}
		}
	}
	else
	{
		for(uint32_t j = 0; j < stepsY; ++j)
		for(uint32_t i = 0; i < stepsX; ++i)
		{
			uint32_t _0 = j * incrementX + i, _1 = _0 + 1;  // _2, _3
			uint32_t _2 = _0 + incrementX,    _3 = _2 + 1;  // _0, _1
			
			indices.push_back(_2);
			indices.push_back(_0);
			indices.push_back(_3);
			
			indices.push_back(_3);
			indices.push_back(_0);
			indices.push_back(_1);
		}
	}
	
	assert(indices.size() == count);
	return indices;
}
/*
std::string Grid::toString() const
{
	std::ostringstream stream;
	stream.imbue(std::locale("C"));
	stream << std::fixed;
	stream.precision(6);
	
	constexpr char _ = ' ';
	stream << "vertex array:" << '\n';
	for(int32_t i = 0, count = size.row * size.column; i < count; ++i)
	{
		const vec3f& vertex = at(i);
		stream << '(' << vertex.x << ',' << _ << vertex.y << ',' << _ << vertex.z << ')' << _;
		
		if(i != 0 && i % size.column == 0)
			stream << '\n';
	}

	stream << "index array:" << '\n';
	const size_t breakSize = size.column * 2 + 2;
	for(size_t i = 0, size = indices.size(); i < size; ++i)
	{
		if(i % (breakSize - 2) == 0)  // before last two degenerated indices.
			stream << _ << _;
		stream << indices[i] << _;
		
		if(i % breakSize == 0)
			stream << '\n';
	}
	
	return stream.str();
}
*/
