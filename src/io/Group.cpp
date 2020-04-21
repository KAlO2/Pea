#include "io/Group.h"

#include <cassert>

using namespace pea;

Group::Group(GroupType type):
		type(type),
		smooth(0)
{
}

Group::Group(const Group& group):
		type(group.type),
		materialName(group.materialName),
		smooth(group.smooth),
		indices(group.indices)
{
}

Group& Group::operator=(const Group& other)
{
	assert(this->type == other.type);
	if(this != &other)
	{
		this->materialName = other.materialName;
		this->smooth = other.smooth;
		this->indices = other.indices;
	}
	
	return *this;
}
/*
Group::Group(GroupType type, size_t capacity):
		type(type),
		smooth(0)
{
	indices.reserve(capacity);
}

Group(Group&& group):
		type(group.type),
		materialName(std::move(group.materialName))
		smooth(group.smooth),
		indices(std::move(group.indices))
{
	
}

void Group::setIndices(const std::vector<uint32_t>& indices)
{
	this->indices = indices;
}

void Group::setIndices(std::vector<uint32_t>&& indices)
{
	this->indices = std::move(indices);
}
*/
void Group::clear()
{
	materialName.clear();
	smooth = false;
	indices.clear();
}

