#include "GMFile.h"
#include "utils.h"

uint32_t SectionStrings::calcSize(GMFile* gmf, uint32_t offset) {
	// Calc size
	header.size = sizeof(uint32_t) + sizeof(uint32_t) * count;

	for (string str : strings) {
		header.size += sizeof(uint32_t) + str.length() + 1;
	}

	// alignment to 128
	uint32_t align = 128 - ((offset + header.size) % 128);
	header.size += align;

	return header.size;
}

bool SectionStrings::fromFile(GMFile* gmf, Header &h, FILE* f, uint32_t offset) {
	header = h;

	fseek(f, offset, 0);

	fread(&count, sizeof(uint32_t), 1, f);

	// Read offset
	for (int i = 0; i < count; i++) {
		uint32_t str_offset;
		fread(&str_offset, sizeof(uint32_t), 1, f);
		offsets.push_back(str_offset);
	}

	// Read strings
	for (int i = 0; i < count; i++) {
		fseek(f, offsets[i], 0);

		uint32_t length;
		fread(&length, sizeof(uint32_t), 1, f);

		char* str = new char[length+1];
		fread(str, sizeof(char), length, f);
		str[length] = 0;
		strings.push_back(str);
		delete str;
	}

	return true;
}

bool SectionStrings::fromDir(GMFile* gmf, Header &h, string basepath) {
	header = h;

	string json_file = Util::join(basepath, "data.json");

	rapidjson::Document document;

	ifstream fin(json_file, ios::binary);
	if (!fin.is_open()) {
		cout << "File not found: " << json_file << endl;
		return false;
	}

	rapidjson::IStreamWrapper isw(fin);
	document.ParseStream(isw);

	// Load data
	auto list = document.GetArray();
	count = list.Size();
	for (int i = 0; i < list.Size(); i++) {
		strings.push_back(list[i].GetString());
	}

	return true;
}

bool SectionStrings::toFile(GMFile* gmf, FILE* f, uint32_t offset) {
	fseek(f, offset, 0);

	fwrite(&count, sizeof(uint32_t), 1, f);

	offsets.clear();
	fseek(f, offset + sizeof(uint32_t) + sizeof(uint32_t) * count, 0);
	for (int i = 0; i < count; i++) {
		uint32_t str_offset = ftell(f);
		string str = strings[i];
		uint32_t length = str.size();
		fwrite(&length, sizeof(uint32_t), 1, f);
		fwrite(str.c_str(), sizeof(char), length + 1, f);
		offsets.push_back(str_offset);
	}

	uint32_t offset_offset = offset + sizeof(uint32_t);
	for (int i = 0; i < count; i++) {
		fseek(f, offset_offset, 0);
		uint32_t str_offset = offsets[i];
		fwrite(&str_offset, sizeof(uint32_t), 1, f);
		offset_offset += sizeof(uint32_t);
	}
	
	return true;
}

bool SectionStrings::toDir(GMFile* gmf, string section_path) const {
	string json_file = Util::join(section_path, "data.json");

	FILE* fp = fopen(json_file.c_str(), "wb");

	char writeBuffer[65536];
	rapidjson::FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));

	rapidjson::Writer<rapidjson::FileWriteStream> writer(os);
	writer.StartArray();
	for (string str : strings) {
		writer.String(str.c_str());
	}
	writer.EndArray();

	fclose(fp);
	return true;
}