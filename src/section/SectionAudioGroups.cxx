#include "GMFile.h"
#include "utils.h"

uint32_t SectionAudioGroups::calcSize(GMFile* gmf, uint32_t offset) {
	this->offset = offset;

	// Calc size
	header.size = sizeof(uint32_t) + sizeof(uint32_t) * count * 2;

	return header.size;
}

bool SectionAudioGroups::fromFile(GMFile* gmf, Header &h, FILE* f, uint32_t offset) {
	header = h;

	fseek(f, offset, 0);

	fread(&count, sizeof(uint32_t), 1, f);

	// Read offset
	vector<uint32_t> offsets;
	for (int i = 0; i < count; i++) {
		uint32_t info_offset;
		fread(&info_offset, sizeof(uint32_t), 1, f);
		offsets.push_back(info_offset);
	}

	// Read groups
	for (int i = 0; i < count; i++) {
		fseek(f, offsets[i], 0);

		AudioGroupInfo info;
		fread(&info.nameOffset, sizeof(uint32_t), 1, f);

		infos.push_back(info);
	}

	return true;
}
bool SectionAudioGroups::fromDir(GMFile* gmf, Header &h, string section_path) {
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
		AudioGroupInfo info;
		info.nameID = list[i]["nameID"].GetInt();

		info.nameOffset = 0;
		infos.push_back(info);
	}

	return true;
}

bool SectionAudioGroups::toFile(GMFile* gmf, FILE* f) {
	fseek(f, offset, 0);

	fwrite(&count, sizeof(uint32_t), 1, f);

	// Write offset
	uint32_t info_offset = offset + sizeof(uint32_t) + sizeof(uint32_t) * count;
	for (int i = 0; i < count; i++) {
		fwrite(&info_offset, sizeof(uint32_t), 1, f);
		info_offset += sizeof(uint32_t);
	}

	return true;
}

bool SectionAudioGroups::toDir(GMFile* gmf, string section_path) const {
	string json_file = Util::join(section_path, "data.json");

	FILE* fp = fopen(json_file.c_str(), "wb");

	char writeBuffer[65536];
	rapidjson::FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));

	rapidjson::Writer<rapidjson::FileWriteStream> writer(os);
	writer.StartArray();
	for (AudioGroupInfo info : infos) {
		writer.StartObject();

		writer.Key("name");
		writer.String(info.name.c_str());

		writer.Key("nameID");
		writer.Int(info.nameID);

		writer.EndObject();
	}
	writer.EndArray();

	fclose(fp);
	return true;
}

// Link strings and audio group
bool SectionAudioGroups::linkFrom(GMFile* gmf) {
	SectionStrings* sectionStrings = (SectionStrings*)gmf->getSection(HEADER_STRINGS);

	for (AudioGroupInfo& info : infos) {
		if (!sectionStrings->getStringByOffset(info.nameOffset, info.nameID, info.name)) {
			cout << "No valid name found for audio group " << info.nameOffset << endl;
			return false;
		}
	}

	return true;
}

bool SectionAudioGroups::linkTo(GMFile* gmf, FILE* f) {
	SectionStrings* sectionStrings = (SectionStrings*)gmf->getSection(HEADER_STRINGS);

	// Write audio group info
	fseek(f, offset + sizeof(uint32_t) + sizeof(uint32_t) * count, 0);
	for (int i = 0; i < count; i++) {
		infos[i].nameOffset = sectionStrings->getOffset(infos[i].nameID);
		fwrite(&infos[i].nameOffset, sizeof(uint32_t), 1, f); // padding
	}
	return true;
}

bool SectionAudioGroups::getGroup(const int id, AudioGroupInfo& group) const {
	if (id >= infos.size()) return false;
	group = infos[id];
	return true;
}
