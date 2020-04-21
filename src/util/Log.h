#ifndef PEA_UTIL_LOG_H_
#define PEA_UTIL_LOG_H_

namespace pea {

class Log
{
public:
	enum Level
	{
		LEVEL_VERBOSE,  // lowest
		LEVEL_DEBUG,
		LEVEL_INFO,
		LEVEL_WARNING,
		LEVEL_ERROR,
		LEVEL_FATAL,    // highest
	};

private:
	static const int BUFF_SIZE;

	Level level;
	bool  colorful;

private:
	Log();
	~Log();

	Log(const Log& log) = delete;
	Log& operator=(const Log& log) = delete;

	static char levelToChar(Level level);

public:
	Level getLevel() { return level; }

	/** @brief Defines the minimum log level to be displayed.
	 */
	void setLevel(Level level);

	void setColorful(bool colorful) { Log::colorful = colorful; }

	static Log& instance();


	void write(Level level, const char* tag, const char* text);
	void print(Level level, const char* tag, const char* format, ...);

	void v(const char* tag, const char* format, ...);
	void d(const char* tag, const char* format, ...);
	void i(const char* tag, const char* format, ...);
	void w(const char* tag, const char* format, ...);
	void e(const char* tag, const char* format, ...);
	void f(const char* tag, const char* format, ...);
};

// singleton
extern Log& slog;

}  // namespace pea
#endif  // PEA_UTIL_LOG_H_
