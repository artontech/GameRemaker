
#include <assert.h>
#include <fstream>
#include <stdio.h>

#include "GMFile.h"
#include "utils.h"

// constructor
GMFile::GMFile() {
}

// New scetion
Section* GMFile::newSection(HeaderName name) {
	Section* section = NULL;

	switch (name) {
	case HEADER_FORM: {
		break;
	}
	case HEADER_GENERAL: {
		section = new SectionGeneral();
		break;
	}
	case HEADER_STRINGS: {
		section = new SectionStrings();
		break;
	}
	case HEADER_TEXTURES: {
		section = new SectionTextures();
		break;
	}
	default:
		section = new SectionUnknown();
		break;
	}

	return section;
}

// Load GameMaker file
bool GMFile::fromFile(string input) {
	FILE* f = fopen(input.c_str(), "rb");
	if (f == NULL) {
		cout << "Failed to open: " << input << endl;
		return false;
	}

	fread(&form, sizeof(Header), 1, f);
	assert(form.name == HEADER_FORM);
	uint32_t total_size = sizeof(Header) + form.size;

	uint32_t offset = sizeof(Header);

	while (offset < total_size) {
		fseek(f, offset, 0);
		Header header;
		fread(&header, sizeof(Header), 1, f);
		offset += sizeof(Header);

		Section* section = newSection(header.name);
		if (!section->fromFile(this, header, f, offset)) return false;
		sections.push_back(section);

		cout << "Offset: " << offset << ", " << section->toString() << endl;

		offset += header.size;
	}

	fclose(f);
	return true;
}

// Load from directory
bool GMFile::fromDir(string input) {
	string json_file = Util::join(input, "project.json");

	rapidjson::Document document;

	ifstream fin(json_file, ios::binary);
	if (!fin.is_open()) {
		cout << "File not found: " << json_file << endl;
		return false;
	}

	rapidjson::IStreamWrapper isw(fin);
	document.ParseStream(isw);

	// Set form
	form.name = HEADER_FORM;

	// Load data
	assert(document.HasMember("sections"));
	assert(document["sections"].IsArray());
	for (const auto& sec : document["sections"].GetArray()) {
		Header header;
		for (const auto& field : sec.GetObject()) {
			string field_name = field.name.GetString();

			if (field_name == "name") {
				Parser32 name;
				name.uint32 = 0;
				strcpy_s(name.chr, 5, field.value.GetString());

				header.name = (HeaderName) name.uint32;
			}
		}

		// Create section
		string section_path = Util::join(input, header.getName());

		Section* section = newSection(header.name);
		if (!section->fromDir(this, header, section_path)) return false;
		sections.push_back(section);

		cout << section->toString() << endl;
	}

	fin.close();
	return true;
}

// Pack data to file
bool GMFile::toFile(string output) {
	FILE* f = fopen(output.c_str(), "wb");
	if (f == NULL) {
		cout << "Failed to open: " << output << endl;
		return false;
	}

	// Compile
	form.size = 0;
	uint32_t offset = sizeof(Header);
	for (Section* section : sections) {
		// For header
		form.size += sizeof(Header);
		offset += sizeof(Header);

		// Calc size
		uint32_t section_size = section->calcSize(this, offset);
		form.size += section_size;
		offset += section_size;
	}

	// Dump to file
	fwrite(&form, sizeof(form), 1, f);
	offset = sizeof(Header);
	for (Section* section : sections) {
		// Dump header
		fseek(f, offset, 0);
		fwrite(&section->header, sizeof(Header), 1, f);
		offset += sizeof(Header);

		// Dump data
		if (!section->toFile(this, f, offset)) return false;
		offset += section->header.size;
	}

	fclose(f);
	return true;
}

// Unpack data to target directory
bool GMFile::toDir(string output) {
	string json_file = Util::join(output, "project.json");

	FILE* f = fopen(json_file.c_str(), "wb");
	if (f == NULL) {
		cout << "Failed to open: " << json_file << endl;
		return false;
	}

	char writeBuffer[65536];
	rapidjson::FileWriteStream os(f, writeBuffer, sizeof(writeBuffer));

	rapidjson::Writer<rapidjson::FileWriteStream> writer(os);
	writer.StartObject();

	writer.Key("size");
	writer.Uint(form.size);

	writer.Key("sections");
	writer.StartArray();
	for (Section* section : sections) {
		string section_name = section->header.getName();
		string section_path = Util::join(output, section_name);
		filesystem::create_directory(section_path);

		// Write section data to dir
		if (!section->toDir(this, section_path)) return false;

		// Write to root json
		writer.StartObject();
		writer.Key("name");
		writer.String(section_name.c_str());
		writer.Key("size");
		writer.Uint(section->header.size);
		writer.EndObject();
	}
	writer.EndArray();

	writer.EndObject();

	fclose(f);
	return true;
}