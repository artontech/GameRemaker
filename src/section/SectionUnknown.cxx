
#include "GMFile.h"
#include "utils.h"

uint32_t SectionUnknown::calcSize(GMFile* gmf, uint32_t offset) {
	return header.size;
}

bool SectionUnknown::fromFile(GMFile* gmf, Header &h, FILE* f, uint32_t offset) {
	header = h;

	fseek(f, offset, 0);

	data = new uint8_t[header.size];
	fread(data, sizeof(uint8_t), header.size, f);
	
	return true;
}

bool SectionUnknown::fromDir(GMFile* gmf, Header &h, string basepath) {
	header = h;

	string data_file = Util::join(basepath, "data.dat");
	FILE * f = fopen(data_file.c_str(), "rb");
	if (f == NULL) {
		cout << "Failed to open: " << data_file << endl;
		return false;
	}
	fseek(f, 0L, SEEK_END);
	header.size = ftell(f);
	rewind(f);

	data = new uint8_t[header.size];
	fread(data, sizeof(uint8_t), header.size, f);
	return true;
}

bool SectionUnknown::toFile(GMFile* gmf, FILE* f, uint32_t offset) {
	fseek(f, offset, 0);

	fwrite(data, sizeof(uint8_t), header.size, f);

	return true;
}

bool SectionUnknown::toDir(GMFile* gmf, string section_path) const {
	string section_name = header.getName();

	string data_file = Util::join(section_path, "data.dat");
	FILE * f = fopen(data_file.c_str(), "wb");
	fwrite(data, sizeof(uint8_t), header.size, f);
	fclose(f);

	return true;
}
