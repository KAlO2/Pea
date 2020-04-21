#ifndef MODEL_OBJ_PRIVATE_H_
#define MODEL_OBJ_PRIVATE_H_

static const std::string comment("# https://github.com/KAlO2/Pea engine");

// stream can be a std::ostringstream or std::ofstream, seperator _ must be defined ahead.
#define PUT_SCALAR(key, value) stream << key << _ << value << '\n'
#define PUT_VEC2(key, v)       stream << key << _ << v.x << _ << v.y << '\n'
#define PUT_VEC3(key, v)       stream << key << _ << v.x << _ << v.y << _ << v.z << '\n'
#define PUT_STRING(key, str)   do { if(!str.empty()) stream << key << _ << str << '\n'; } while(false)

using namespace pea;

static constexpr bool isSpace(char ch)
{
	return (ch == ' ') || (ch == '\t');
}

static constexpr bool isNewLine(char ch)
{
	return (ch == '\r') || (ch == '\n') || (ch == '\0');
}

inline bool matchChar(const char* &p, char ch)
{
	bool flag = (*p == ch) && isSpace(p[1]);
	if(flag)
		p += 2;
	return flag;
}

inline bool matchTwoChar(const char* &p, uint16_t ch1ch2)
{
	bool flag = (*reinterpret_cast<const uint16_t*>(p) == ch1ch2) && isSpace(p[2]);
	if(flag)
		p += 3;
	return flag;
};

inline bool matchString(const char* &p, const char* key, size_t len)
{
	bool flag = strncmp(p, key, len) == 0 && isSpace(p[len]);
	if(flag)
		p += (len + 1);
	return flag;
};

#define MATCH_CHAR(ch)            matchChar(p, ch)
#define MATCH_TWO_CHAR(ch1, ch2)  matchTwoChar(p, static_cast<uint16_t>(ch1 | (ch2 << 8)))

// NOTE: str must be char[] type, namely quoted string constant, not char* type.
#define MATCH_STRING(str)         matchString(p, str, sizeof(str) - 1)


#endif  // MODEL_OBJ_PRIVATE_H_
