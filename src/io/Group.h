#ifndef PEA_IO_GROUP_H_
#define PEA_IO_GROUP_H_

#include <cstdint>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace pea {

enum class GroupType: std::uint8_t
{
	VERTEX = 0,
	EDGE   = 1,
	FACE   = 2,
};

class Group
{
private:
	const GroupType type;
	std::string materialName;
	uint32_t smooth;
public:
	std::vector<uint32_t> indices;
	
public:
	Group(GroupType type);
//	Group(Group&& group);

	Group(const Group& group);
	Group& operator=(const Group& other);
	
	GroupType getType() const;
	
	void setMaterial(const std::string& name);
	const std::string& getMaterial() const;
	
	void setSmooth(uint32_t smooth);
	uint32_t getSmooth() const;
	
//	void setIndices(const std::vector<uint32_t>& indices);
//	void setIndices(std::vector<uint32_t>&& indices);

	bool isEmpty() const;
	void clear();
	
	template <typename T>
	static std::vector<T> createIndex(const std::vector<T>& data, std::vector<uint32_t>& index);
	
	template <typename T, typename U>
	static std::vector<T> dropIndex(const std::vector<T>& data, const std::vector<U>& indices);
};

inline GroupType Group::getType() const { return type; }

inline void Group::setMaterial(const std::string& name) { this->materialName = name; }
inline const std::string& Group::getMaterial() const { return materialName; }

inline void Group::setSmooth(uint32_t smooth) { this->smooth = smooth; }
inline uint32_t Group::getSmooth() const { return smooth; }

inline bool Group::isEmpty() const { return indices.empty(); }


/**
 * remove duplicate, and keep order.
 * @param[in] data input data
 * @param[out] indices index data to create
 * @return unique data.
 */
template <typename T>
std::vector<T> Group::createIndex(const std::vector<T>& data, std::vector<uint32_t>& index)
{
	const size_t size = data.size();
	std::vector<T> uniqueData;
	uniqueData.reserve(size);
	index.resize(size);
/*
	auto hash = [](const T& v) { return std::hash<T>{}(v); };
	auto compare = [](const T& lhs, const T& rhs) { return lhs == rhs; };
	std::unordered_map<T, uint32_t, decltype(hash), decltype(compare)> map(size, hash, compare);
*/
	std::unordered_map<T, uint32_t> map(size);
	for(size_t i = 0, j = 0; i < size; ++i)
	{
		const T& datum = data[i];
		auto it = map.find(datum);
		if(it != map.end())
		{
			index[i] = it->second;
			continue;
		}
		
		uniqueData.push_back(datum);
		map.emplace_hint(it, datum, j);
		index[i] = j;
		++j;
	}
	
	assert(uniqueData.size() <= size);
	return uniqueData;
}

template <typename T, typename U>
std::vector<T> Group::dropIndex(const std::vector<T>& data, const std::vector<U>& indices)
{
	static_assert(std::is_integral<U>::value && !std::is_signed<U>::value, "U must be an unsigned integer type");
	if(data.empty())
		return {};
	
	size_t indexSize = indices.size();
	std::vector<T> _data;
	_data.reserve(indexSize);
	
	for(const U& index: indices)
		_data.push_back(data[index]);
	
	return _data;
}

}  // namespace pea
#endif  // PEA_IO_GROUP_H_
