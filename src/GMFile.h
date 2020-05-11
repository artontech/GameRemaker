#pragma once
#include "stdafx.h"

#include <fstream>
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


	Section* newSection(HeaderName name);
public:
	GMFile();
	bool fromFile(string input);
	bool fromDir(string input);
	bool toFile(string output);
	bool toDir(string output);
};

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// Section parent, pure virtual
class Section {
public:
	Header header;

	virtual string toString() const;
	virtual uint32_t calcSize(GMFile* gmf, uint32_t offset) = 0;
	virtual bool fromFile(GMFile* gmf, Header &h, FILE* f, uint32_t offset) = 0;
	virtual bool fromDir(GMFile* gmf, Header &h, string basepath) = 0;
	virtual bool toFile(GMFile* gmf, FILE* f, uint32_t offset) = 0;
	virtual bool toDir(GMFile* gmf, string section_path) const = 0;
};

// General section
class SectionGeneral : public Section {
public:
	GEN8 gen8;
	uint32_t* numbers;

	bool toJsonFile(string path) const;
	virtual uint32_t calcSize(GMFile* gmf, uint32_t offset);
	virtual bool fromFile(GMFile* gmf, Header &h, FILE* f, uint32_t offset);
	virtual bool fromDir(GMFile* gmf, Header &h, string basepath);
	virtual bool toFile(GMFile* gmf, FILE* f, uint32_t offset);
	virtual bool toDir(GMFile* gmf, string section_path) const;
};

// Strings section
class SectionStrings : public Section {
public:
	uint32_t count;
	vector<uint32_t> offsets;
	vector<string> strings;

	virtual uint32_t calcSize(GMFile* gmf, uint32_t offset);
	virtual bool fromFile(GMFile* gmf, Header &h, FILE* f, uint32_t offset);
	virtual bool fromDir(GMFile* gmf, Header &h, string basepath);
	virtual bool toFile(GMFile* gmf, FILE* f, uint32_t offset);
	virtual bool toDir(GMFile* gmf, string section_path) const;
};

// Unknown section
class SectionUnknown : public Section {
public:
	uint8_t* data;

	virtual uint32_t calcSize(GMFile* gmf, uint32_t offset);
	virtual bool fromFile(GMFile* gmf, Header &h, FILE* f, uint32_t offset);
	virtual bool fromDir(GMFile* gmf, Header &h, string basepath);
	virtual bool toFile(GMFile* gmf, FILE* f, uint32_t offset);
	virtual bool toDir(GMFile* gmf, string section_path) const;
};

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
