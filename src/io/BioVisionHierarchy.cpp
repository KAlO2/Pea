#include "io/BioVisionHierarchy.h"

#include <cassert>
#include <cstring>
#include <fstream>
#include <ostream>
#include <set>

#include "scene/Skeleton.h"
#include "util/Log.h"

using namespace pea;

// BVH format doesn't specify token case.
const std::string HIERARCHY = "HIERARCHY";
const std::string ROOT = "ROOT";  // first joint
const std::string JOINT = "JOINT";
const std::string OFFSET = "OFFSET";
const std::string CHANNELS = "CHANNELS";
const std::string POSITION = "position";
const std::string ROTATION = "rotation";
const std::string END = "End";
const std::string SITE = "Site";
const std::string MOTION = "MOTION";
const std::string FRAMES = "Frames";
const std::string FRAME = "Frame";
const std::string TIME = "Time";
//const std::string LEFT_PARENTHESIS = "{";
//const std::string RIGHT_PARENTHESIS = "}";

static inline bool equal(const std::string& s1, const std::string& s2)
{
	// case insensative compare
	// TODO: strcasecmp is in STL or not?
	return strcasecmp(s1.c_str(), s2.c_str()) == 0;
}

static const char* TAG = "BioVisionHierarchy";

BioVisionHierarchy::BioVisionHierarchy():
		frameCount(0),
		frameTime(0),
		valueCountPerFrame(0)
{
}

static int32_t indent(std::ifstream& stream)
{
	std::string token;
	stream >> token;
	if(token == "{")
		return 1;
	else if(token == "}")
		return -1;
	else
		return 0;
}

static bool parseOffset(std::ifstream& stream, vec3f& offset)
{
	if(!(stream >> offset.x))
		return false;
	if(!(stream >> offset.y))
		return false;
	if(!(stream >> offset.z))
		return false;
	return true;
}

static bool parseChannel(std::ifstream& stream, std::vector<BioVisionHierarchy::Channel>& channels)
{
	int32_t channelCount = 0;
	stream >> channelCount;
	using Channel = BioVisionHierarchy::Channel;
	if(channelCount <= 0 || channelCount > Channel::CHANNEL_COUNT)
	{
		slog.e(TAG, "invalid channel count number %d", channelCount);
		return false;
	}
	uint32_t flag = 0;
	static_assert(Channel::CHANNEL_COUNT < 32);
	
	channels.reserve(channelCount);
	std::string token;
	for(int32_t i = 0; i < channelCount; ++i)
	{
		if(!(stream >> token))
			return false;
		
		Channel channel;
#if 0
		if(token == "Xposition")
			channel = Channel::POSITION_X;
		else if(token == "Yposition")
			channel = Channel::POSITION_Y;
		else if(token == "Zposition")
			channel = Channel::POSITION_Z;
		
		else if(token == "Xrotation")
			channel = Channel::ROTATION_X;
		else if(token == "Yrotation")
			channel = Channel::ROTATION_Y;
		else if(token == "Zrotation")
			channel = Channel::ROTATION_Z;
		
		else
		{
			slog.w(TAG, "invalid channel. name=%s", token.c_str());
			return false;
		}
#else
		if(token.size() != 9)  // [X|YZ] + [position|rotation]
		{
			slog.w(TAG, "invalid channel. name=%s", token.c_str());
			return false;
		}
		
		const char& axis = token[0];
		if(axis < 'X' || axis > 'Z')
		{
			slog.w(TAG, "invalid axis. axis=%c", axis);
			return false;
		}

		const char* transform = token.data() + 1;
		if(equal(transform, POSITION))
			channel = static_cast<Channel>(Channel::POSITION_X + (axis - 'X'));
		else if (equal(transform, ROTATION))
			channel = static_cast<Channel>(Channel::ROTATION_X + (axis - 'X'));
		else
		{
			slog.w(TAG, "invalid transform. transform=%s", transform);
			return false;
		}
#endif
		if((flag & (1 << channel)) == 0)
			flag |= 1 << channel;
		else
		{
			slog.e(TAG, "two same channels in one %s. channel=%d", CHANNELS.c_str(), channel);
			return false;
		}
		channels.push_back(channel);
	}
	
	return true;
}

bool BioVisionHierarchy::load(const std::string& path)
{
	assert(!hasHierarchyData() && !hasMotionData());
	
	std::ifstream stream(path, std::ios::in);
	if(!stream.is_open())
	{
		slog.e(TAG, "could not open path for reading. path=%s", path.c_str());
		return false;
	}
	
	std::string section;
	stream >> section;
	if(!equal(section, HIERARCHY))
	{
		slog.w(TAG, "expect %s segment but get %s", HIERARCHY.c_str(), section.c_str());
		return false;
	}
	
	bool rootFound = false;
	int32_t depth = -1;
	int32_t stepsBack = 0;
	while(stream.peek() != EOF)
	{
		Joint joint;
		
		std::string token;
		stream >> token;
		if(equal(token, JOINT) || equal(token, ROOT))
		{
			if(equal(token, ROOT))
			{
				if(rootFound)
				{
					slog.e(TAG, "two root joints encountered");
					return false;
				}
				else
					rootFound = true;
			}
			
			stream >> joint.name;
			int32_t parenthesis = indent(stream);
			if(parenthesis != 1)
			{
				slog.e(TAG, "expect indentation { after ROOT/JOINT <name>");
				return false;
			}
			else
				++depth;
			
			stream >> token;
			if(!equal(token, OFFSET) || !parseOffset(stream, joint.offset))
			{
				slog.w(TAG, "%s expect %s token, got %s", (rootFound? ROOT: JOINT).c_str(), OFFSET.c_str(), token.c_str());
				return false;
			}
			
			stream >> token;
			if(!equal(token, CHANNELS) || !parseChannel(stream, joint.channels))
			{
				slog.w(TAG, "%s expect %s token, got %s", (rootFound? ROOT: JOINT).c_str(), CHANNELS.c_str(), token.c_str());
				return false;
			}
			
			valueCountPerFrame += joint.channels.size();
			if(stepsBack > 0)
			{
				int index = static_cast<int32_t>(joints.size()) - 1;
				for(; stepsBack > 0; --stepsBack)
					index = joints[index].parent;
				joint.parent = index;
			}
			else
				joint.parent = static_cast<int32_t>(joints.size()) - 1;
			
			joints.push_back(joint);
			slog.v(TAG, "add joint #%zu %s, parent=%d", joints.size() - 1, joint.name.c_str(), joint.parent);
		}
		else if(equal(token, END))
		{
			stream >> token;
			if(!equal(token, SITE))
			{
				slog.w(TAG, "expect %s before %s", END.c_str(), SITE.c_str());
				return false;
			}
			
			int32_t parenthesis = indent(stream);
			if(parenthesis != 1)
			{
				slog.e(TAG, "expect indentation { after End <name>");
				return false;
			}
			else
				++depth;
			
			stream >> token;
			if(!equal(token, OFFSET) || !parseOffset(stream, joint.offset))
			{
				slog.w(TAG, "End expect %s token", OFFSET.c_str());
				return false;
			}
			
			joint.parent = static_cast<int32_t>(joints.size()) - 1;  // apparently
			joints.push_back(joint);
			slog.v(TAG, "add End Site #%zu, parent=%d", joints.size() - 1, joint.parent);
			
			while(true)
			{
				int ch = stream.get();
				if(isspace(ch))
					continue;
				else if(ch == '}')
				{
					--depth;
					++stepsBack;
					if(depth < 0)
						goto MOTION_SECTION;
				}
				else
				{
					stream.putback(ch);
					break;
				}
			}
		}
		else
		{
			slog.e(TAG, "unknown token, token=%s", token.c_str());
			return false;
		}
	}
	
MOTION_SECTION:
	stream >> section;
	if(!equal(section, MOTION))
	{
		slog.w(TAG, "expect %s segment", MOTION.c_str());
		return false;
	}
	
	for(int32_t i = 0; i < 2; ++i)
	{
		std::string token;
		stream >> token;
		if(stream.fail() || token.empty()) 
			return false;
		
		// allow space before or after colon
		if(equal(token, FRAME))  // Frame Time:
			stream >> token;
		
		if(token[token.size() - 1] == ':')
			token.pop_back();
		else
		{
			while(true)
			{
				int ch = stream.get();
				if(isspace(ch))
					continue;
				
				if(ch == ':')
					break;
				else
				{
					slog.w(TAG, "expect colon, got %c", ch);
					return false;
				}
			}
		}
		
		if(equal(token, FRAMES))
			stream >> frameCount;
		else if(equal(token, TIME))
			stream >> frameTime;
		else
		{
			slog.w(TAG, "unknown token. token=%s", token.c_str());
			return false;
		}
	}
	
	if(frameCount <= 0 || frameTime <= 0)
	{
		slog.e(TAG, "invalid frame paramters. frameCount=%d, frameTime=%f", frameCount, frameTime);
		return false;
	}
	
	slog.v(TAG, "valueCountPerFrame=%d", valueCountPerFrame);
	int32_t valueCount = valueCountPerFrame * frameCount;
	values.reserve(valueCount);
	for(int32_t i = 0; i < valueCount; ++i)
	{
		float value;
		stream >> value;
		if(stream.fail())
			return false;
		values.push_back(value);
	}
	
	return true;
}

void BioVisionHierarchy::scale(float positionFactor, float rotationFactor/* = 1.0*/)
{
	assert(positionFactor > 0);
	const bool changeRotation = rotationFactor != 1.0F;
	
	int32_t index = 0;
	for(Joint& joint: joints)
	{
		joint.offset *= positionFactor;
		
		int32_t channelCount = static_cast<int32_t>(joint.channels.size());
		for(int32_t i = 0; i < channelCount; ++i)
		{
			float& value = values[index + i];
			if(joint.channels[i] <= Channel::POSITION_Z)
				value *= positionFactor;
			else if(changeRotation)
				value *= rotationFactor;
		}
		index += channelCount;
	}
}
/*
// pre-order traversal => level order traversal
static std::map<std::string, uint32_t> buildTree(const std::vector<Joint>& joints)
{
	const uint32_t size = joints.size();
	std::set<uint32_t> parents, children, remaining;
	for(uint32_t i = 0; i < size; ++i)
	{
		if(joints[i].parent >= 0)  // [[likely]]
			remaining.push(i);
		else
			parents.push(i);
	}
	assert(parents.size() == 1);  // only one root node
	
	std::vector<uint32_t> skeleton;
	while(!remaining.empty())
	{
		// move children of set_parent from set_remaining to set_children
		for(auto it = remaining.begin(); it != remaining.end(); )
		{
			uint32_t childIndex = *it;
			uint32_t parentIndex = joints[childIndex].parent;
//			if(parents.contains(parentIndex))  // C++20 way
			if(parents.find(parentIndex) != parents.end())
			{
				it = remaining.erase(it);
				children.push(childIndex);
			}
		}
		
		
	}
}
*/

bool BioVisionHierarchy::save(const std::string& path, int32_t precision/* = 6*/) const
{
	std::ofstream file(path);
	if(!file.is_open())
		return false;

	file.imbue(std::locale("C"));
	// int32_t precision = std::numeric_limits<long double>::digits10 + 1;
#if 0  // fixed precision
	file << std::fixed;
	file.precision(precision);
	auto shortify = [](const float& x) { return x; };
#else
	std::ostringstream oss;
	// remove trailing zeros of a float number
	auto shortify = [&oss, &precision](float x) -> std::string
	{
		// reuse std::ostringstream
		oss.clear();
		oss.str("");
		
		oss << std::fixed << std::setprecision(precision) << x;
		std::string str = oss.str();
		size_t dot = str.find_first_of('.');
		if(dot != std::string::npos)
		{
			size_t nz = str.find_last_not_of('0');
			if(nz + 2 > str.size())
				return str;
			else
				return str.substr(0, nz + 2);
		}
		else
			return str;
	};
#endif
	if(joints.empty())
		return false;
	
	file << HIERARCHY << '\n';
	constexpr char _ = ' ';
	
	int32_t depth = 0;
	constexpr int32_t N = 128;
	std::string indentation(N, '\t');
	auto indent = [&file, &indentation, &depth]()
	{
		if(depth > N) [[unlikely]]
			indentation.append(depth - N + 8, '\t');
		file.write(indentation.data(), depth);
	};
	
	const int32_t size = static_cast<int32_t>(joints.size());
	for(int32_t i = 0; i < size; ++i)
	{
		const Joint& joint = joints[i];
		
		indent();
		bool leaf = false;
		if(joint.parent < 0)
			file << ROOT;
		else if(i + 1 >= size || joints[i + 1].parent != i)
		{
			leaf = true;
			file << "End Site" << '\n';
		}
		else
			file << JOINT;
		
		if(!leaf)
			file << _ << joint.name << '\n';
		
		indent();
		file << '{' << '\n';
		++depth;
		
		const vec3f& offset = joint.offset;
		indent();
		file << OFFSET << _ << shortify(offset.x) << _ << shortify(offset.y) << _ << shortify(offset.z) << '\n';
		
		if(leaf)
		{
			if(i + 1 >= size)  // last
			{
				while(depth > 0)
				{
					--depth;
					indent();
					file << '}' << '\n';
				}
			}
			else
			{
				int32_t j = i;
				int32_t nextParent = joints[i + 1].parent;
				while(j != nextParent)
				{
					--depth;
					assert(depth > 0);
					indent();
					file << '}' << '\n';
					j = joints[j].parent;
				}
			}
		}
		else
		{
			const std::vector<Channel>& channels = joint.channels;
			indent();
			file << CHANNELS << _ << channels.size() << _;
			for(size_t c = 0, channelCount = channels.size(); c < channelCount; ++c)
			{
				int32_t channel = static_cast<int32_t>(channels[c]);
				assert(c < CHANNEL_COUNT);
				file << static_cast<char>('X' + channel % 3) << (channel < 3? POSITION: ROTATION);
				if(c + 1U < channelCount)  // discard trailing space.
					file << _;
			}
			file << '\n';
		}
	}
	
	file << MOTION << '\n';
	file << FRAMES << ':' << _ << frameCount << '\n';
	file << FRAME << _ << TIME << ':' << frameTime << '\n';
	for(int32_t k = 0; k < frameCount; ++k)
	{
		int32_t base = k * valueCountPerFrame;
		for(int32_t i = 0; i < valueCountPerFrame; ++i)
			file << shortify(values[base + i]) << '\t';
		
		if(k + 1 < frameCount)
			file << '\n';
	}

	file.close();
	return true;
}

void BioVisionHierarchy::clear(bool keepHierarchy)
{
	// motion data
	frameCount = 0;
	frameTime = 0;
	valueCountPerFrame = 0;
	values.clear();
	
	// hierarchy data
	if(!keepHierarchy)
		joints.clear();
}

bool BioVisionHierarchy::hasHierarchyData() const
{
	return !joints.empty();
}

bool BioVisionHierarchy::hasMotionData() const
{
	return frameCount > 0 && frameTime > 0 && valueCountPerFrame > 0;
	// frameCount * valueCountPerFrame == values.size()
}
/*
mat4f BioVisionHierarchy::calculateLocalTransform(int32_t boneIndex, int32_t frameIndex) const
{

}
*/
void BioVisionHierarchy::calculateGlobalTransforms(std::vector<mat4f>& transforms, int32_t frame) const
{
	assert(0 <= frame && frame < frameCount);
	int32_t index = frame * valueCountPerFrame;
	const int32_t size = static_cast<int32_t>(joints.size());
	transforms.resize(size);
	for(int32_t i = 0; i < size; ++i)
	{
		const Joint& joint = joints[i];
		const std::vector<Channel>& channels = joint.channels;
		const int32_t channelCount = static_cast<int32_t>(channels.size());
		mat4f transform(1.0);
		vec3f translation = joint.offset;
		for(int32_t i = channelCount - 1; i >= 0; --i)
		{
			Channel channel = channels[i];
			if(channel == ROTATION_X)
				transform.rotateX(values[index++]);
			else if(channel == ROTATION_Y)
				transform.rotateY(values[index++]);
			else if(channel == ROTATION_Z)
				transform.rotateZ(values[index++]);
			else if(POSITION_X <= channel && channel <= POSITION_Z)
				translation[channel - POSITION_X] += values[index++];
			else
				assert(false);  // unreachable code
		}
		
		transform.translate(translation);
		
		// turn local transform to global transform
		assert(joint.parent < i);  // the order that BVH keeps.
		if(joint.parent >= 0)
			transforms[i] = transform * transforms[joint.parent];
		else
			transforms[i] = transform;  // root
	}
}
/*
void BioVisionHierarchy::exportRestPose(std::vector<Bone>& bones) const
{
	const int32_t size = 2;//static_cast<int32_t>(joints.size());
	bones.resize(size);  // exclude root joint, now each bone has a parent bone.
	mat4f rootTransform(1.0f);
	for(int32_t i = 0; i < size; ++i)
	{
		const Joint& joint = joints[i];
		
		Bone bone;
		bone.name = joint.name;
		bone.head = vec3f(0.0);
		bone.tail = joint.offset;
		bone.parent = joint.parent;
		
		mat4f local = joint.rest.inverse();
		if(joint.parent >= 0)
			bone.global = local * bones[joint.parent].global;
		else
			bone.local = local;
		std::cout << "bone" << i << ", name=" << bone.name << ", parent=" << bone.parent << '\n' << bone.rest << "local\n" << bone.local << "\nglobal\n" << bone.global << "\n\n";
		bones.push_back(bone);
	}
}
*/
void BioVisionHierarchy::exportFramePose(std::vector<Bone>& bones, int32_t frameIndex) const
{
	assert(0 <= frameIndex && frameIndex < frameCount);
	int32_t index = frameIndex * valueCountPerFrame;
	const int32_t size = 3;//static_cast<int32_t>(joints.size());
	bones.reserve(size);  // exclude root joint, now each bone has a parent bone.
	mat4f rootTransform(1.0f);
	for(int32_t i = 0; i < size; ++i)
	{
		const Joint& joint = joints[i];
		const std::vector<Channel>& channels = joint.channels;
		const int32_t channelCount = static_cast<int32_t>(channels.size());
		std::cout << "bone" << i << ", name=" << joint.name << ", parent=" << joint.parent << '\n';
		mat4f transform(1.0);
		vec3f translation = joint.offset;
		// TODO: rotation value is local(relative to parent) or global in .bvh file?
		// rotation ZXY means left multiplication first Y, then X, last Z, namely from right to left.
		for(int32_t i = channelCount - 1; i >= 0; --i)
		{
			const float& value = values[index +i];
			std::cout << "c" << i << " " << value << '\t';
			Channel channel = channels[i];
			if(channel == ROTATION_X)
				transform.rotateX(value);
			else if(channel == ROTATION_Y)
				transform.rotateY(value);
			else if(channel == ROTATION_Z)
				transform.rotateZ(value);
			else if(POSITION_X <= channel && channel <= POSITION_Z)
				translation[channel - POSITION_X] += value;
			else
				assert(false);  // unreachable code
		}
		index += channelCount;
		std::cout << "\ntranslation=" << translation << '\n';// << transform << '\n';
		transform.translate(translation);

		Bone bone;
		bone.name = joint.name;
		bone.head = vec3f(0.0);
		bone.tail = joint.offset;
		bone.parent = joint.parent;

		bone.local = transform;
		if(joint.parent >= 0)  [[likely]]
			bone.global = transform * bones[joint.parent].global;
		else
			bone.global = transform;
		
		std::cout << "local\n" << bone.local << "\nglobal\n" << bone.global << "\n\n";
		bones.push_back(bone);
	}
	std::cout << "index=" << index << ", right=" << (frameIndex + 1) * valueCountPerFrame << '\n';
//	assert(index == (frameIndex + 1) * valueCountPerFrame);
	// root is not a bone.
//	bones.erase(bones.begin());
}

