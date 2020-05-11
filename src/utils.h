#pragma once

#include <algorithm>
#include <string>
#include <direct.h>

using namespace std;

class Util {
public:
	// Get current directory
	static string cwd() {
		return getcwd(NULL, 0);
	}

	// Extract parent path
	static string basepath(const string& filepath) {
		size_t pos = max(filepath.find_last_of('/', 0), filepath.find_last_of('\\', 0));
		if (pos == string::npos) return filepath;
		return filepath.substr(0, pos + 1);
	}

	// Join path
	static string join(const string& path1, const string& path2) {
		char separator;

#ifdef _WIN32
		separator = '\\';
#else
		separator = '/';
#endif

		return path1 + separator + path2;
	}
};