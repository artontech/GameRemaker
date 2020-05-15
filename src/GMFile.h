#pragma once
#include "stdafx.h"

#include <fstream>
#include <map>
#include <vector>

#include "rapidjson/document.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/writer.h"

#include "structures.h"

// Placeholder
class Section;

// GameMaker file
class GMFile {
private:
	Header form;
	vector<Section*> sections;

	bool linkFrom(const HeaderName name);
	bool linkTo(const HeaderName name, FILE* f);
	Section* newSection(const HeaderName name);
public:
	GMFile();
	Section* getSection(const HeaderName name);
	bool fromFile(string input);
	bool fromDir(string input);
	bool toFile(string output);
	bool toDir(string output);
};

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// Section parent, pure virtual
class Section {
public:
	uint32_t offset;
	Header header;

	virtual string toString() const;
	virtual uint32_t calcSize(GMFile* gmf, uint32_t offset) = 0;
	virtual bool fromFile(GMFile* gmf, Header &h, FILE* f, uint32_t offset) = 0;
	virtual bool fromDir(GMFile* gmf, Header &h, string section_path) = 0;
	virtual bool toFile(GMFile* gmf, FILE* f) = 0;
	virtual bool toDir(GMFile* gmf, string section_path) const = 0;
	virtual bool linkFrom(GMFile* gmf) = 0;
	virtual bool linkTo(GMFile* gmf, FILE* f) = 0;
};

// General section
class SectionGeneral : public Section {
private:
	GEN8 gen8;
	uint32_t* numbers;
public:
	bool toJsonFile(string path) const;
	virtual uint32_t calcSize(GMFile* gmf, uint32_t offset);
	virtual bool fromFile(GMFile* gmf, Header &h, FILE* f, uint32_t offset);
	virtual bool fromDir(GMFile* gmf, Header &h, string section_path);
	virtual bool toFile(GMFile* gmf, FILE* f);
	virtual bool toDir(GMFile* gmf, string section_path) const;
	virtual bool linkFrom(GMFile* gmf);
	virtual bool linkTo(GMFile* gmf, FILE* f);
};

// Strings section
class SectionStrings : public Section {
private:
	uint32_t count;
	vector<uint32_t> offsets;
	map<uint32_t, int> offsetMap;
	vector<string> strings;
public:
	virtual uint32_t calcSize(GMFile* gmf, uint32_t offset);
	virtual bool fromFile(GMFile* gmf, Header &h, FILE* f, uint32_t offset);
	virtual bool fromDir(GMFile* gmf, Header &h, string section_path);
	virtual bool toFile(GMFile* gmf, FILE* f);
	virtual bool toDir(GMFile* gmf, string section_path) const;
	virtual bool linkFrom(GMFile* gmf);
	virtual bool linkTo(GMFile* gmf, FILE* f);
	bool getStringByOffset(const uint32_t offset, int& id, string& str) const;
	uint32_t getOffset(const int id) const;
};

// Textures section
class SectionTextures : public Section {
private:
	vector<uint32_t> offsets;
	uint32_t count;
	vector<PNGFile*> png_files;
public:
	virtual uint32_t calcSize(GMFile* gmf, uint32_t offset);
	virtual bool fromFile(GMFile* gmf, Header &h, FILE* f, uint32_t offset);
	virtual bool fromDir(GMFile* gmf, Header &h, string section_path);
	virtual bool toFile(GMFile* gmf, FILE* f);
	virtual bool toDir(GMFile* gmf, string section_path) const;
	virtual bool linkFrom(GMFile* gmf);
	virtual bool linkTo(GMFile* gmf, FILE* f);
};

// Sounds section
class SectionSounds : public Section {
private:
	uint32_t count;
	vector<SoundInfo> infos;
public:
	virtual uint32_t calcSize(GMFile* gmf, uint32_t offset);
	virtual bool fromFile(GMFile* gmf, Header &h, FILE* f, uint32_t offset);
	virtual bool fromDir(GMFile* gmf, Header &h, string section_path);
	virtual bool toFile(GMFile* gmf, FILE* f);
	virtual bool toDir(GMFile* gmf, string section_path) const;
	virtual bool linkFrom(GMFile* gmf);
	virtual bool linkTo(GMFile* gmf, FILE* f);
	bool getSoundByAudioID(const int audioID, SoundInfo& sound) const;
};

// AudioGroups section
class SectionAudioGroups : public Section {
private:
	uint32_t count;
	vector<AudioGroupInfo> infos;
public:
	virtual uint32_t calcSize(GMFile* gmf, uint32_t offset);
	virtual bool fromFile(GMFile* gmf, Header &h, FILE* f, uint32_t offset);
	virtual bool fromDir(GMFile* gmf, Header &h, string section_path);
	virtual bool toFile(GMFile* gmf, FILE* f);
	virtual bool toDir(GMFile* gmf, string section_path) const;
	virtual bool linkFrom(GMFile* gmf);
	virtual bool linkTo(GMFile* gmf, FILE* f);
	bool getGroup(const int id, AudioGroupInfo& group) const;
};

// Audio section
class SectionAudio : public Section {
private:
	vector<uint32_t> offsets;
	uint32_t count;
	vector<AudioFile> audio_files;
public:
	virtual uint32_t calcSize(GMFile* gmf, uint32_t offset);
	virtual bool fromFile(GMFile* gmf, Header &h, FILE* f, uint32_t offset);
	virtual bool fromDir(GMFile* gmf, Header &h, string section_path);
	virtual bool toFile(GMFile* gmf, FILE* f);
	virtual bool toDir(GMFile* gmf, string section_path) const;
	virtual bool linkFrom(GMFile* gmf);
	virtual bool linkTo(GMFile* gmf, FILE* f);
};

// Unknown section
class SectionUnknown : public Section {
private:
	uint8_t* data;
public:
	virtual uint32_t calcSize(GMFile* gmf, uint32_t offset);
	virtual bool fromFile(GMFile* gmf, Header &h, FILE* f, uint32_t offset);
	virtual bool fromDir(GMFile* gmf, Header &h, string section_path);
	virtual bool toFile(GMFile* gmf, FILE* f);
	virtual bool toDir(GMFile* gmf, string section_path) const;
	virtual bool linkFrom(GMFile* gmf);
	virtual bool linkTo(GMFile* gmf, FILE* f);
};

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
