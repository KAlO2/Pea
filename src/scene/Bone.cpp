#include "scene/Bone.h"

using namespace pea;

Bone::Bone():
		rest(1.0f),
		local(1.0f),
		global(1.0f),
		head(0, 0, 0),// 0.1),
		tail(0, 0, 1),// 0.05),
		parent(-1)
{
	
}

void Bone::reset()
{
	rest = mat4f(1.0);
	local = mat4f(1.0);  //.reset();
	global = mat4f(1.0);  //.reset();
	head = vec3f(0, 0, 0);// 0.1),
	tail = vec3f(0, 0, 1);// 0.05),
	parent = -1;
}

float Bone::length() const
{
	return (tail - head).length();
}
/*
mat4f Bone::getLocalTransform() const
{
	return transform.getTransform();
}

mat4f Bone::getGlobalTransform() const
{
	mat4f m = transform.getTransform();
	for(Bone* p = parent; p != nullptr; p = p->parent)
		m *= parent->transform.getTransform();
	
	return m;
}

void Bone::render(const mat4f& viewProjection) const
{
	
}
*/
