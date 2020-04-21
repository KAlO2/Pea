#ifndef PEA_IO_FILE_SYSTEM_H_
#define PEA_IO_FILE_SYSTEM_H_

#include <string>
#include <vector>

namespace pea {

class FileSystem
{
public:
	static const char SEPERATOR;
	static const int PATH_MAX_LENGTH;

	static std::string getWorkingDirectory();
	
	/**
	 * CMake out of source build
	 */
	static std::string getRootDirectory();
	
	/**
	 * Get the path relative to the pea working directory
	 */
	static std::string getRelativePath(const std::string& path);
private:
	/**
	 * All the searching paths reside in here. The lower index is searched first.
	 */
	std::vector<std::string> search_path;

private:
	FileSystem();
	~FileSystem();

public:
	static FileSystem& instace();

	static std::vector<std::string> listFiles(const std::string& directory);
	
	/**
	 * Load file @code filename content into memory, it's useful to pass
	 * shader source code to OpenGL.
	 * Note that the C version(char*) need to be free'd after used.
	 *
	 * @param filename the filename
	 * @return the string content of the file
	 */
	static char*       load(const char* filename);
	static std::string load(const std::string& filename);

	/**
	 * @return the component following the final '/' or "." if none is available.
	 * Trailing '/' characters are not counted as part of the pathname.
	 */
	static std::string basename(const std::string& path);

	/**
	 * @return returns the string up to, but not including, the final '/'.
	 *
	 * note that concatenating the string returned by dirname(), a "/", and the string
	 * returned by basename() yields a complete pathname.
	 */
	static std::string dirname(const std::string& path);

	static std::string realpath(const std::string& path);

	static void normalize(std::string& path);
	static std::string normalize(const std::string& path);

	static bool isAbsolute(const std::string& path);
	static bool exists(const std::string& path);

	bool addSearchPath(const std::string& path, bool front = true);
};

}  // namespace pea
#endif  // PEA_IO_FILE_SYSTEM_H_
