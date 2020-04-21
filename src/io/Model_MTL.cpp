#include "io/Model_MTL.h"

#include <cstring>
#include <fstream>

#include "io/Model_OBJ.private.h"
#include "io/FileSystem.h"
#include "io/Type.h"
#include "opengl/Texture.h"
#include "util/compiler.h"
#include "util/Log.h"

static constexpr char KEY_MATERIAL[]            = "newmtl";
static constexpr char KEY_EMISSIVE[]            = "Ke";
static constexpr char KEY_AMBIENT[]             = "Ka";
static constexpr char KEY_DIFFUSE[]             = "Kd";
static constexpr char KEY_SPECULAR[]            = "Ks";
static constexpr char KEY_TRANSMISSION_FILTER[] = "Tf";
static constexpr char KEY_TRANSMITTANCE[]       = "Tr";

static constexpr char KEY_ILLUMINATION[]        = "illum"; ///< enum IlluminationMode
static constexpr char KEY_DISSOLVE[]            = "d";
static constexpr char KEY_DISSOLVE_HALO[]       = "-halo";
static constexpr char KEY_SPECULAR_EXPONENT[]   = "Ns";
static constexpr char KEY_SHARPNESS[]           = "sharpness";
static constexpr char KEY_OPTICAL_DENSITY[]     = "Ni";
static constexpr char KEY_AMBIENT_TEXTURE[]     = "map_Ka";
static constexpr char KEY_DIFFUSE_TEXTURE[]     = "map_Kd";
static constexpr char KEY_SPECULAR_TEXTURE[]    = "map_Ks";
static constexpr char KEY_MAP_NS[]              = "map_Ns";
static constexpr char KEY_MAP_D[]               = "map_d";
static constexpr char KEY_MAP_BUMP[]            = "map_Bump";
static constexpr char KEY_BUMP_TEXTURE[]        = "disp";
static constexpr char KEY_DECAL[]               = "decal";
static constexpr char KEY_BUMP[]                = "bump";
static constexpr char KEY_REFL[]                = "refl";

using namespace pea;


static const char* TAG = "Model_MTL";
/*
const Property Model_MTL::property[Model_MTL::KEY_COUNT] =
{
	PROPERTY_SET("newmtl"   , Type::STRING),

	PROPERTY_SET("Ka"       , Type::VEC3F),
	PROPERTY_SET("Kd"       , Type::VEC3F),
	PROPERTY_SET("Ks"       , Type::VEC3F),
	PROPERTY_SET("Tf"       , Type::VEC3F),
	PROPERTY_SET("illum"    , Type::FLOAT),
	PROPERTY_SET("d"        , Type::FLOAT),
	PROPERTY_SET("d -halo"  , Type::FLOAT),
	PROPERTY_SET("Ns"       , Type::INT32),  // [0, 1000]
	PROPERTY_SET("sharpness", Type::INT32),  // [0, 1000], default 60
	PROPERTY_SET("Ni"	    , Type::FLOAT),

	PROPERTY_SET("map_Ka"   , Type::STRING),
	PROPERTY_SET("map_Kd"   , Type::STRING),
	PROPERTY_SET("map_Ks"   , Type::STRING),
};

const Model_MTL Model_MTL::EMPTY("");
*/
/*

bool Model_MTL::check(const char*& str, Key key)
{
	Property prop = Model_MTL::property[key];
	bool hasKey = !strncmp(str, prop.key, prop.skip) && isspace(str[prop.skip]);
	if(hasKey)
		str += prop.skip + 1;  // key and a space
	return hasKey;
}

*/

Model_MTL::Model_MTL(const std::string& path) noexcept(false):
		path(path)
{
	std::ifstream stream(path, std::ios::in | std::ios::binary);;
	if(!stream.is_open())
		throw new std::invalid_argument("can open path for reading. path=" + path);
	
	std::shared_ptr<Material> material;

	auto emplaceMaterial = [&material, &materials = this->materials]()
	{
		if(material.get() == nullptr)  // [[unlikely]]
			return;
		
		// flush previous material
		const std::string& name = material->name;
		if(!name.empty())
			materials.emplace(name, material);  // insertion
		material.reset();
	};
	
	const size_t N = 4096;
	char buffer[N];
	const char* p = nullptr;
	while(stream.peek() != EOF)
	{
		stream.getline(buffer, N);

		// trim newline '\r\n' or '\n'
		size_t len = std::strlen(buffer);
		if(len > 0 && (buffer[len - 1] == '\n'))
		{
			--len;
			buffer[len] = '\0';
		}
		if(len > 0 && (buffer[len - 1] == '\r'))
		{
			--len;
			buffer[len] = '\0';
		}

		// skip leading space.
		p = buffer + std::strspn(buffer, " \t");

		if(buffer[0] == '\0')  // empty line
			continue;
		if(buffer[0] == '#')  // comment line
			continue;

		if(MATCH_STRING("newmtl"))  // "newmtl"
		{
			emplaceMaterial();  // flush previous material
			
			std::string name(p);
//			slog.v(TAG, "newmtl %s", p);
			material = std::make_shared<Material>(name);  // start a new material
		}
		else if(material == nullptr)
			slog.w(TAG, "expect newmtl <name> before material attributes");
		else if(MATCH_TWO_CHAR('K', 'a'))
			material->ambient = Type::parseFloat3(p);
		else if(MATCH_TWO_CHAR('K', 'd'))
			material->diffuse = Type::parseFloat3(p);
		else if(MATCH_TWO_CHAR('K', 's'))
			material->specular = Type::parseFloat3(p);
		else if(MATCH_TWO_CHAR('K', 't'))
			material->transmittance = Type::parseFloat3(p);
		else if(MATCH_TWO_CHAR('N', 'i'))
			material->ior = Type::parseFloat(p);
		else if(MATCH_TWO_CHAR('K', 'e'))
			material->emissive = Type::parseFloat3(p);
		else if(MATCH_TWO_CHAR('N', 's'))
			material->shininess = Type::parseFloat(p);
		else if(MATCH_STRING(KEY_ILLUMINATION))  // "illum"
		{
			uint32_t illum = Type::parseUint(p);
			material->illum = static_cast<IlluminationMode>(illum);
		}
		else if(MATCH_CHAR('d'))
		{
			if(material->dissolve > 0)
				slog.w(TAG, "overwrite quantity 'Tr' from quantity 'd'");
			material->dissolve = Type::parseFloat(p);
		}
		else if(MATCH_TWO_CHAR('T', 'r'))  // transparency
		{
			// defines the transparency of the material to be alpha. The default is 0.0 (not 
			// transparent at all). The quantities d and Tr are the opposites of each other, and 
			// specifying transparency or nontransparency is simply a matter of user convenience.
			float transparency = Type::parseFloat(p);
			if(material->dissolve > 0)
				slog.w(TAG, "overwrite quantity 'd' from quantity 'Tr'");
			material->dissolve = 1.0f - transparency;
		}
		else if(MATCH_STRING(KEY_AMBIENT_TEXTURE))  // "map_Ka"
			material->ambient_texname = p;
		else if(MATCH_STRING(KEY_DIFFUSE_TEXTURE))  // "map_Kd"
			material->diffuse_texname = p;
		else if(MATCH_STRING(KEY_SPECULAR_TEXTURE))  // "map_Ks"
			material->specular_texname = p;
		else if(MATCH_STRING(KEY_MAP_NS))  // "map_Ns"
			material->specular_highlight_texname = p;
		else if(MATCH_STRING(KEY_MAP_BUMP))  // "map_Bump"
			material->bump_texname = p;
		else if(MATCH_STRING(KEY_MAP_D))  // "map_d"
			material->alpha_texname = p;
		else if(MATCH_STRING(KEY_BUMP))  // "bump"
			material->bump_texname = p;
		else if(MATCH_STRING(KEY_BUMP_TEXTURE))  // "disp"
			material->displacement_texname = p;
		else  // unknown parameter goes here
		{
			size_t length = strcspn(p, " \t");
			std::string key(p, length);
			p += length;  // key
			p += strspn(p, " \t");  // skip space

			const char* end = &p[strlen(p) - 1];
			while((*end == ' ') || (*end == '\t'))
				--end;

			length = end - p + 1;
			if(*p)
			{
				std::string value(p, length);
				material->unknownParameters[key] = value;
			}
		}
	}
	
	emplaceMaterial();  // flush last material
}

Model_MTL::Model_MTL(Model_MTL&& other):
		path(other.path),
		materials(std::move(other.materials))
{
}
/*
Model_MTL& Model_MTL::operator =(Model_MTL&& other)
{
	if(this != &other)
	{
		this->path = other.path;
		this->materials = std::move(other.materials)
	}
	
	return *this;
}
*/

std::unordered_map<std::string, std::shared_ptr<Texture>> Model_MTL::loadTexture() const
{
	std::unordered_map<std::string, std::shared_ptr<Texture>> textureMap;
	std::string dir = FileSystem::dirname(path) + FileSystem::SEPERATOR;
	Texture::Parameter parameter;
	
	auto addTexture = [&dir, &parameter, &textureMap](Texture::Type type, const std::string& textureName)
	{
		if(textureName.empty())
			return;
		
		std::shared_ptr<Texture> texture = std::make_shared<Texture>();
		std::string path = dir + textureName;
		if(textureMap.find(textureName) != textureMap.end())
			return;
		slog.v(TAG, "loadTexture() %s", path.c_str());
		if(texture->load(path, parameter))
		{
			texture->setType(type);
			textureMap.emplace(textureName, texture);
		}
	};
	
	for(const std::pair<std::string, std::shared_ptr<Material>>& pair: materials)
	{
		Material* material = pair.second.get();
		assert(material);
		
		addTexture(Texture::Type::AMBIENT, material->ambient_texname);
		addTexture(Texture::Type::DIFFUSE, material->diffuse_texname);
		addTexture(Texture::Type::SPECULAR, material->specular_texname);
	}
	
	return textureMap;
}

size_t Model_MTL::getSize() const
{
	return materials.size();
}

bool Model_MTL::contains(const std::string& name)
{
	return materials.find(name) != materials.end();
}
/*
Material Model_MTL::at(const std::string& name)
{
	auto it = materials.find(name);
	if(it != materials.end())
		return Material();
	else
		return it->second;
}
*/
std::shared_ptr<Material> Model_MTL::at(const std::string& name) const
{
	auto it = materials.find(name);
	assert(it != materials.end());
//		return std::shared_ptr<Material>();
	return it->second;
}

bool Model_MTL::save(const std::string& path) const
{
	std::ofstream file(path);
	if(!file.is_open())
		return false;

	file.imbue(std::locale("C"));
	file << std::fixed;
	file.precision(6);
	
	constexpr char _ = ' ';
	file << comment << '\n';
	file << '#' << _ << "Material Count:" << _ << materials.size() << '\n';
	file << '\n';

	for(const std::pair<std::string, std::shared_ptr<Material>>& pair: materials)
		file << Model_MTL::toString(pair.second.get()) << '\n';

	file.close();
	return true;
}

std::string Model_MTL::toString(const Material* material)
{
	if(material == nullptr || material->name.empty())
		return {};
	
	std::ostringstream stream;
	stream.imbue(std::locale("C"));
	stream << std::fixed;
	stream.precision(6);

	constexpr char _ = ' ';
	constexpr vec3f ZERO(0.0F);
	stream << KEY_MATERIAL << _ << material->name << '\n';

	PUT_VEC3(KEY_AMBIENT, material->ambient);
	PUT_VEC3(KEY_DIFFUSE, material->diffuse);
	PUT_VEC3(KEY_SPECULAR, material->specular);
	if(material->transmittance != ZERO)
		PUT_VEC3(KEY_TRANSMITTANCE, material->transmittance);
	if(material->emissive != ZERO)
		PUT_VEC3(KEY_EMISSIVE, material->emissive);

	PUT_SCALAR(KEY_SPECULAR_EXPONENT, material->shininess);
	PUT_SCALAR(KEY_OPTICAL_DENSITY,   material->ior);
	PUT_SCALAR(KEY_DISSOLVE,          material->dissolve);
	PUT_SCALAR(KEY_ILLUMINATION,      underlying_cast<IlluminationMode>(material->illum));

	PUT_STRING(KEY_AMBIENT_TEXTURE,  material->ambient_texname);
	PUT_STRING(KEY_DIFFUSE_TEXTURE,  material->diffuse_texname);
	PUT_STRING(KEY_SPECULAR_TEXTURE, material->specular_texname);
	PUT_STRING(KEY_MAP_BUMP,         material->bump_texname);
	PUT_STRING(KEY_MAP_NS,           material->specular_highlight_texname);
	/*
#if __cplusplus >= 201703L // structured binding is added in C++17
	for(const auto& [key, value]: material->unknownParameters)
		stream << key << _ << value << '\n';
#else
	for(const std::pair<std::string, std::string>& pair: material->unknownParameters)
	{
		const std::string& key = pair.first;
		const std::string& value = pair.second;
		stream << key << _ << value << '\n';
	}
#endif
	*/
	return stream.str();
}

std::string Model_MTL::toString() const
{
	std::ostringstream os;
	for(const std::pair<std::string, std::shared_ptr<Material>>& pair: materials)
	{
//		const std::string& name = pair.first;
		const Material* material = pair.second.get();
		assert(material != nullptr);
		os << Model_MTL::toString(material) << '\n';
	}
	
	return os.str();
}

void Model_MTL::clear()
{
//	for(const std::pair<std::string, Material*>& pair: materials)
//		delete pair.second;
	materials.clear();
}
