#include "GMFile.h"
#include "utils.h"

uint32_t SectionStrings::calcSize(GMFile* gmf, uint32_t offset) {
	this->offset = offset;

	// Calc size
	uint32_t str_offset = offset + sizeof(uint32_t) + sizeof(uint32_t) * count;

	// Calc string offset
	for (string str : strings) {
		offsets.push_back(str_offset); // record offset for toFile()
		str_offset += sizeof(uint32_t) + str.length() + 1;
	}

	str_offset += Util::align(str_offset, 128);
	header.size = str_offset - offset;

	return header.size;
}

bool SectionStrings::fromFile(GMFile* gmf, Header &h, FILE* f, uint32_t offset) {
	header = h;

	fseek(f, offset, 0);

	fread(&count, sizeof(uint32_t), 1, f);

	// Read offset
	for (int i = 0; i < count; i++) {
		uint32_t entry_offset;
		fread(&entry_offset, sizeof(uint32_t), 1, f);
		offsets.push_back(entry_offset);
	}

	// Read strings
	for (int i = 0; i < count; i++) {
		fseek(f, offsets[i], 0);

		uint32_t str_offset = offsets[i] + sizeof(uint32_t);
		offsetMap[str_offset] = i;

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

bool SectionStrings::fromDir(GMFile* gmf, Header &h, string section_path) {
	header = h;

	string json_file = Util::join(section_path, "data.json");

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

	fin.close();
	return true;
}

bool SectionStrings::toFile(GMFile* gmf, FILE* f) {
	fseek(f, offset, 0);

	fwrite(&count, sizeof(uint32_t), 1, f);

	// Write offset
	for (int i = 0; i < count; i++) {
		fwrite(&offsets[i], sizeof(uint32_t), 1, f);
	}

	// Write strings
	for (int i = 0; i < count; i++) {
		fseek(f, offsets[i], 0);
		uint32_t length = strings[i].size();
		fwrite(&length, sizeof(uint32_t), 1, f);
		fwrite(strings[i].c_str(), sizeof(char), length + 1, f);
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

bool SectionStrings::linkFrom(GMFile* gmf) {
	return true;
}

bool SectionStrings::linkTo(GMFile* gmf, FILE* f) {
	return true;
}

bool SectionStrings::getStringByOffset(const uint32_t offset, int& id, string& str) const {
	auto iter = offsetMap.find(offset);
	if (iter == offsetMap.end()) return false;
	if (iter->second >= strings.size()) return false;

	id = iter->second;
	str = strings[iter->second];
	return true;
}

uint32_t SectionStrings::getOffset(const int id) const {
	return offsets[id] + sizeof(uint32_t); // skip string length
}
