#pragma once

#include <algorithm>
#include <string>
#include <direct.h>
#include <stdint.h>

using namespace std;

class Util {
public:
	// Get current directory
	static string cwd() {
		return getcwd(NULL, 0);
	}

	// Extract parent path
	static string section_path(const string& filepath) {
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

	// Swap uint32 bytes
	static uint32_t swap(uint32_t v) {
		return (v & 0xFF000000) >> 24 | (v & 0x00FF0000) >> 8 | (v & 0x0000FF00) << 8 | (v & 0x000000FF) << 24;
	}

	// Align
	static uint32_t align(const uint32_t addr, const uint32_t base) {
		return (base - (addr % base)) % base;
	}

	static string unicodeToUTF8(const wstring &wstr) {
		wstring_convert<codecvt_utf8<wchar_t>> wcv;
		return wcv.to_bytes(wstr);
	}

	static wstring UTF8ToUnicode(const string &str) {
		wstring_convert<codecvt_utf8<wchar_t>> wcv;
		return wcv.from_bytes(str);
	}
};