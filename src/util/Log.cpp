#include "util/Log.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(__ANDROID__)
#	include <android/log.h>
#elif defined(_WIN32)
#	ifndef WIN32_LEAN_AND_MEAN
#		define WIN32_LEAN_AND_MEAN
#	endif
#	include <windows.h>
#endif

namespace pea {

const char* TAG = "Log";
const int Log::BUFF_SIZE = 4096;
Log& slog = Log::instance();

#ifdef _WIN32
// SetConsoleTextAttribute function
// https://msdn.microsoft.com/en-us/library/windows/desktop/ms686047(v=vs.85).aspx
static const WORD COLOR[] =
{
	FOREGROUND_RED | FOREGROUND_GREEN,  // yellow for unknown
	0,  // black for verbose
	FOREGROUND_GREEN | FOREGROUND_BLUE,  // cyan for debug
	FOREGROUND_GREEN,  // green for info
	FOREGROUND_RED | FOREGROUND_BLUE,  // magenta for warning
	FOREGROUND_RED,  // red for error
	FOREGROUND_RED | BACKGROUND_INTENSITY  // white for fatal
};
#else
// http://en.wikipedia.org/wiki/ANSI_escape_code#Colors
static const char COLOR[] =
{
	33,  // yellow for unknown
	30,  // black for verbose
	36,  // cyan for debug
	32,  // green for info
	35,  // magenta for warning
	31,  // red for error
	37   // white for fatal
};
#endif

Log::Log():
		level(Log::LEVEL_VERBOSE),
		colorful(true)
{
}

Log::~Log()
{
	// To reset colors to their defaults, use \x1b[39;49m
	// or reset all attributes with \x1b[0m.
	printf("\x1b[0m");
}

void Log::setLevel(Level level)
{
	if(level < LEVEL_VERBOSE || level > LEVEL_FATAL)
		slog.w(TAG, "Log level %d not in range [%d-%d], ignored.",
				level, LEVEL_VERBOSE, LEVEL_FATAL);
	else
		this->level = level;
}

Log& Log::instance()
{
	static Log log;
	return log;
}

char Log::levelToChar(Level level)
{
	switch(level)
	{
	case LEVEL_VERBOSE: return 'V';
	case LEVEL_DEBUG:   return 'D';
	case LEVEL_INFO:    return 'I';
	case LEVEL_WARNING: return 'W';
	case LEVEL_ERROR:   return 'E';
	case LEVEL_FATAL:   return 'F';
	default:
		assert(false);
		return '?';
	}
}

void Log::print(Level level, const char* tag, const char* format, ...)
{
	va_list ap;
	char buff[BUFF_SIZE];

	va_start(ap, format);
	vsnprintf(buff, BUFF_SIZE, format, ap);
	va_end(ap);

#ifdef __ANDROID__
	__android_log_write(level, tag, buff);
#else
	write(level, tag, buff);
#endif
}

void Log::write(Level level, const char* tag, const char* text)
{
	if(level < LEVEL_VERBOSE || level > LEVEL_FATAL)
		slog.w(TAG, "Log level %d not in range [%d-%d], ignored.",
				level, LEVEL_VERBOSE, LEVEL_FATAL);

	if(!tag)
		tag = "";

	if(level < this->level)
		return;  // silence

	char ch = levelToChar(level);
	if(colorful)
	{
#ifdef _WIN32
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), COLOR[level]);
#else
		printf("\e[1;%im%c/%s: %s\n", static_cast<int>(COLOR[level]), ch, tag, text);
#endif
	}
	else
		printf("%c/%s: %s\n", ch, tag, text);

}

#define LOG(NAME, LEVEL)                                       \
	void Log::NAME(const char* tag, const char* format, ...)   \
	{                                                          \
		va_list ap;                                            \
		char buff[BUFF_SIZE];                                  \
	                                                           \
		va_start(ap, format);                                  \
		vsnprintf(buff, BUFF_SIZE, format, ap);                \
		va_end(ap);                                            \
		write(LEVEL, tag, buff);                               \
	}                                                          \

LOG(v, LEVEL_VERBOSE)
LOG(d, LEVEL_DEBUG)
LOG(i, LEVEL_INFO)
LOG(w, LEVEL_WARNING)
LOG(e, LEVEL_ERROR)
//LOG(f, LEVEL_FATAL)

// terminate program for fatal level.
void Log::f(const char* tag, const char* format, ...)
{
	va_list ap;
	char buff[BUFF_SIZE];

	va_start(ap, format);
	vsnprintf(buff, BUFF_SIZE, format, ap);
	va_end(ap);
	write(LEVEL_FATAL, tag, buff);

	// crash intentionally
	int* ABADB01 = nullptr;
	*ABADB01 = 0xABADB01;
	exit(EXIT_FAILURE);

}

}  // namespace pea
