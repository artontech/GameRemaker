
#include "GMFile.h"
#include "utils.h"

uint32_t SectionUnknown::calcSize(GMFile* gmf, uint32_t offset) {
	this->offset = offset;
	return header.size;
}

bool SectionUnknown::fromFile(GMFile* gmf, Header &h, FILE* f, uint32_t offset) {
	header = h;

	fseek(f, offset, 0);

	data = new uint8_t[header.size];
	fread(data, sizeof(uint8_t), header.size, f);
	
	return true;
}

bool SectionUnknown::fromDir(GMFile* gmf, Header &h, string section_path) {
	header = h;

	string data_file = Util::join(section_path, "data.dat");
	FILE * fp = fopen(data_file.c_str(), "rb");
	if (fp == NULL) {
		cout << "Failed to open: " << data_file << endl;
		return false;
	}
	fseek(fp, 0L, SEEK_END);
	header.size = ftell(fp);
	rewind(fp);

	data = new uint8_t[header.size];
	fread(data, sizeof(uint8_t), header.size, fp);
	fclose(fp);
	return true;
}

bool SectionUnknown::toFile(GMFile* gmf, FILE* f) {
	fseek(f, offset, 0);

	fwrite(data, sizeof(uint8_t), header.size, f);

	return true;
}

bool SectionUnknown::toDir(GMFile* gmf, string section_path) const {
	string section_name = header.getName();

	string data_file = Util::join(section_path, "data.dat");
	FILE * fp = fopen(data_file.c_str(), "wb");
	fwrite(data, sizeof(uint8_t), header.size, fp);
	fclose(fp);

	return true;
}

bool SectionUnknown::linkFrom(GMFile* gmf) {
	return true;
}

bool SectionUnknown::linkTo(GMFile* gmf, FILE* f) {
	return true;
}
