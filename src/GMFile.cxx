
#include <assert.h>
#include <fstream>
#include <stdio.h>

#include "GMFile.h"
#include "utils.h"

// constructor
GMFile::GMFile() {
}

Section* GMFile::getSection(const HeaderName name) {
	for (Section* section : sections) {
		if (name == section->header.name) {
			return section;
		}
	}
	return NULL;
}

bool GMFile::linkFrom(const HeaderName name) {
	Section* section = getSection(name);
	if (NULL == section) return false;
	return section->linkFrom(this);
}

bool GMFile::linkTo(const HeaderName name, FILE* f) {
	Section* section = getSection(name);
	if (NULL == section) return false;
	return section->linkTo(this, f);
}

// New scetion
Section* GMFile::newSection(const HeaderName name) {
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
	case HEADER_SOUNDS: {
		section = new SectionSounds();
		break;
	}
	case HEADER_AUDIO_GROUPS: {
		section = new SectionAudioGroups();
		break;
	}
	case HEADER_AUDIO: {
		section = new SectionAudio();
		break;
	}
	case HEADER_FONTS: {
		section = new SectionFonts();
		break;
	}
	case HEADER_TEXTURE_PAGES: {
		section = new SectionTexturePages();
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
	if (NULL == f) {
		cout << "Failed to open: " << input << endl;
		return false;
	}

	// Load data
	fread(&form, sizeof(Header), 1, f);
	assert(HEADER_FORM == form.name);
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

	// Link audio group name
	linkFrom(HEADER_AUDIO_GROUPS);

	// Link sounds
	linkFrom(HEADER_SOUNDS);

	// Link audio
	linkFrom(HEADER_AUDIO);

	// Link fonts
	linkFrom(HEADER_FONTS);

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

		// Get name
		Parser32 name;
		name.uint32 = 0;
		strcpy_s(name.chr, 5, sec["name"].GetString());
		header.name = (HeaderName)name.uint32;

		// Create section
		string section_path = Util::join(input, header.getName());

		Section* section = newSection(header.name);
		if (!section->fromDir(this, header, section_path)) return false;
		section->padEnd = sec["padEnd"].GetUint();

		sections.push_back(section);
		cout << string(13, '\b') << "Loading: " << section->header.getName();
	}
	cout << endl;

	fin.close();
	return true;
}

// Pack data to file
bool GMFile::toFile(string output) {
	FILE* f = fopen(output.c_str(), "wb");
	if (NULL == f) {
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
		uint32_t section_size = section->calcSize(this, offset) + section->padEnd;
		section->header.size = section_size;
		form.size += section_size;
		offset += section_size;
	}

	// Dump to file
	rewind(f);
	fwrite(&form, sizeof(form), 1, f);
	for (Section* section : sections) {
		cout << section->toString() << endl;

		// Dump header
		fseek(f, section->offset - sizeof(Header), 0);
		fwrite(&section->header, sizeof(Header), 1, f);

		// Dump data
		if (!section->toFile(this, f)) return false;
	}

	// Link audio group name
	linkTo(HEADER_AUDIO_GROUPS, f);

	// Link sounds
	linkTo(HEADER_SOUNDS, f);

	// Link fonts
	linkTo(HEADER_FONTS, f);

	fclose(f);
	return true;
}

// Unpack data to target directory
bool GMFile::toDir(string output) {
	string json_file = Util::join(output, "project.json");

	FILE* f = fopen(json_file.c_str(), "wb");
	if (NULL == f) {
		cout << "Failed to open: " << json_file << endl;
		return false;
	}

	char writeBuffer[65536];
	rapidjson::FileWriteStream os(f, writeBuffer, sizeof(writeBuffer));

	rapidjson::Writer<rapidjson::FileWriteStream> writer(os);
	writer.StartObject();

	writer.Key("sections");
	writer.StartArray();
	for (Section* section : sections) {
		cout << string(12, '\b') << "Saving: " << section->header.getName();

		string section_name = section->header.getName();
		string section_path = Util::join(output, section_name);
		filesystem::create_directory(section_path);

		// Write section data to dir
		if (!section->toDir(this, section_path)) return false;

		// Write to root json
		writer.StartObject();

		writer.Key("name");
		writer.String(section_name.c_str());

		writer.Key("padEnd");
		writer.Uint(0);

		writer.EndObject();
	}
	cout << endl;
	writer.EndArray();

	writer.EndObject();

	fclose(f);
	return true;
}