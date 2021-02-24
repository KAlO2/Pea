#include "physics/Cloth.h"

#include "geometry/Grid.h"
#include "math/Random.h"
#include "physics/SpringConstraint.h"
#include "physics/BendingConstraint.h"
#include "scene/Mesh.h"
#include "util/compiler.h"
#include "util/Log.h"

using namespace pea;

static const char* TAG = "Cloth";
 
Cloth::Cloth(const Model& model, float mass):
		model(model),
		windForce(0, 0, 0)
{
	const std::vector<vec3f>& vertices = model.getVertexData();
	const size_t vertexSize = vertices.size();
	assert(mass > 0 && vertexSize > 0);
	
	float particleMass = mass / vertexSize;
	const vec3f position(0.0F, 0.0F, 0.0F);
	Particle particle(position, mass);
	slog.d(TAG, "mass=%f, particleMass=%f", mass, particleMass);
	
	particles.resize(vertexSize, particle);

	// particles' order are the same with model's vertex order.
	for(size_t i = 0; i < vertexSize; ++i)
	{
		const vec3f& position = vertices[i];
		particles[i].setPosition(position);
		// and set last position to be the same value.
		particles[i].updatePosition(position);  // or particles[i].translate(vec3f(0, 0, 0));
	}
	
	constexpr float damping[SpringTypeCount]   = {0.75, 0.75, 0.65, 0.55};
	constexpr float stiffness[SpringTypeCount] = {0.25, 0.25, 0.35, 0.45};
	
	std::vector<std::vector<uint32_t>> adjacentVerticesArray(vertexSize);
	const std::vector<uint32_t> edgeIndices = model.getEdgeIndices();
	const size_t edgeIndexSize = edgeIndices.size();
	for(size_t i = 0; i < edgeIndexSize; i += 2)
	{
		const uint32_t &_0 = edgeIndices[i], &_1 = edgeIndices[i + 1];
		assert(_0 < vertexSize && _1 < vertexSize);
	
		SpringConstraint* constraint = new SpringConstraint(particles[_0], particles[_1]);
		constraint->setDamping(damping[SpringType::COMPRESSION], damping[SpringType::TENSION]);
		constraint->setStiffness(stiffness[SpringType::COMPRESSION], stiffness[SpringType::TENSION]);
		constraints.emplace_back(constraint);
		
		adjacentVerticesArray[_0].push_back(_1);
		adjacentVerticesArray[_1].push_back(_0);
	}
#if 0  // debug adjacent vertices
	for(size_t i = 0; i < vertexSize; ++i)
	{
		std::vector<uint32_t>& array = adjacentVerticesArray[i];
		std::cout << "v #" << i << " ";
		for(const uint32_t& index: array)
			std::cout << index << ' ';
		std::cout << '\n';
	}
	std::cout << '\n';
#endif

	const std::vector<uint32_t> quadIndices = model.getQuadrilateralIndices();
	const size_t quadIndexSize = quadIndices.size();
	for(size_t i = 0; i < quadIndexSize; i += 4)
	{
		const uint32_t &_0 = quadIndices[i],     &_3 = quadIndices[i + 3];
		const uint32_t &_1 = quadIndices[i + 1], &_2 = quadIndices[i + 2];
		assert(_0 < vertexSize && _1 < vertexSize && _2 < vertexSize && _3 < vertexSize);

		SpringConstraint* constraint = new SpringConstraint(particles[_0], particles[_2]);
		constraint->setDamping(damping[SpringType::SHEAR]);
		constraint->setStiffness(stiffness[SpringType::SHEAR]);
		constraints.emplace_back(constraint);
		
		constraint = new SpringConstraint(particles[_1], particles[_3]);
		constraint->setDamping(damping[SpringType::SHEAR]);
		constraint->setStiffness(stiffness[SpringType::SHEAR]);
		constraints.emplace_back(constraint);
	}
	slog.i(TAG, "vertexSize=%zu, quadIndexSize=%zu", vertexSize, quadIndexSize);
	// std::vector<vec4u> adjacentVertices;
	// std::map<uint32_t, vec3u> adjacentVertices;
/*
	for(size_t i = 0; i < vertexSize; ++i)
	{
		const std::vector<uint32_t>& indices = adjacentVerticesArray[i];
		if(indices.size() != 4)
			continue;
		
		vec3f v[4];
		for(uint8_t k = 0; k < 4; ++k)
			v[k] = (vertices[indices[k]] - vertices[i]).normalize();
		
		uint32_t _0 = 0, _1 = 1;
		float min_cos_angle = 1;  // cos(0) = 1
		for(uint8_t i0 = 0; i0 < 4; ++i0)
		for(uint8_t i1 = i0 + 1; i1 < 4; ++i1)
		{
			float cos_angle = dot(v[i0], v[i1]);
			if(cos_angle < min_cos_angle)
			{
				_0 = i0;
				_1 = i1;
				min_cos_angle = cos_angle;
			}
		}
		
		const float threshold = -0.5;  // std::cos(M_PI * 2 / 3);  // 120
		if(min_cos_angle < threshold)
		{
//slog.v(TAG, "add bending %d-%d-%d", indices[_0], i, indices[_1]);
			BendingConstraint* constraint0 = new BendingConstraint(particles[indices[_0]], particles[i], particles[indices[_1]]);
			constraints.push_back(constraint0);
			
			uint32_t _2 = 0;
			for(; _2 < 4; ++_2)
				if(_0 != _2 && _1 != _2)
					break;
			uint32_t _3 = (0 + 1 + 2 + 3) - _0 - _1 - _2;
			assert(_0 < 4 && _1 < 4 && _2 < 4 && _3 < 4);
			float cos_angle = dot(v[_2], v[_3]);
			if(cos_angle < 0)  // cos(pi / 2) = 0
			{
//slog.v(TAG, "add bending %d-%d-%d", indices[_2], i, indices[_3]);
				BendingConstraint* constraint1 = new BendingConstraint(particles[indices[_2]], particles[i], particles[indices[_3]]);
				constraints.push_back(constraint1);
			}
		}
	}
*/
}

Cloth::~Cloth()
{
	for(const Constraint* constraint: constraints)
		delete constraint;

	constraints.clear();
}

void Cloth::setStiffness(float stiffness)
{
	for(Constraint* constraint: constraints)
		constraint->setStiffness(stiffness);
}
/*
void Cloth::setStiffness(const float stiffness[SpringTypeCount])
{
	for(Constraint* constraint: constraints)
	{
		uint32_t typeIndex = underlying_cast<Constraint::Type>(constraint->getType());
		constraint->setStiffness(stiffness[typeIndex]);
	}
}
*/
void Cloth::setDamping(float damping)
{
	for(Constraint* constraint: constraints)
		constraint->setDamping(damping);
}
/*
void Cloth::setDamping(const float damping[SpringTypeCount])
{
	for(Constraint* constraint: constraints)
	{
		uint32_t typeIndex = underlying_cast<Constraint::Type>(constraint->getType());
		constraint->setDamping(damping[typeIndex]);
	}
}
*/
bool Cloth::pinGroup(const std::string& name)
{
	const Group* group = nullptr;
	bool foundVertexGroup = model.findGroup(name, group) &&
			group->getType() == GroupType::VERTEX;
	
	for(Particle& particle: particles)
		particle.setMovable(true);
	
	if(foundVertexGroup)
	{
		assert(group != nullptr);
		const std::vector<uint32_t>& indices = group->indices;
		for(const uint32_t& index: indices)
			particles[index].setMovable(false);
	}
	return foundVertexGroup;
}

std::string Cloth::getPinnedGroup() const
{
	return pinnedGroupName;
}

void Cloth::calculateForce()
{
	const vec3f g(0, 0, -9.81);  // gravitational acceleration
	
	// Bernoulli's equation states that  for an incompressible, frictionless fluid,
	// P + 0.5 * ρ * v^2 + ρ * g * h = constant
	// where P is the absolute pressure, ρ is the fluid density, v is the velocity of the fluid,
	// h is the height above some reference point, and g is the acceleration due to gravity.
	float pressure = 0.01;  // N/m^2
//	vec3f windForce(Random::emit(), Random::emit(), Random::emit());
	windForce += Random::sphereEmit(pressure);
	
	// how does wind force act on a cloth?
	// external force like wind and gravity
	for(Particle& particle: particles)
	{
		particle.resetForce();
		
//		if(!particle.isMovable())
//			continue;
		
		vec3f gravity = particle.getMass() * g;
		particle.applyForce(gravity/* + windForce*/);
	}
	
	// internal force, spring model
	for(Constraint* constraint: constraints)
		constraint->calculateForce();
}

void Cloth::step(float dt)
{
	assert(dt > 0);
	calculateForce();
	
	// apply force, internal and exteral
	for(Constraint* constraint: constraints)
		constraint->satisfy();
	
	for(Particle& particle: particles)
		particle.step(dt);
	
	// collision detection
	
}

void Cloth::exportVertextexData(std::vector<vec3f>& vertices) const
{
	const size_t vertexSize = particles.size();
	vertices.resize(vertexSize);
	for(size_t i = 0; i < vertexSize; ++i)
		vertices[i] = particles[i].getPosition();
}
