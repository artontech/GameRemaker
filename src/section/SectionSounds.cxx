#include <filesystem>

#include "GMFile.h"
#include "utils.h"

uint32_t SectionSounds::calcSize(GMFile* gmf, uint32_t offset) {
	this->offset = offset;

	// Calc size
	header.size = sizeof(uint32_t) + sizeof(uint32_t) * count;

	// Calc audio file offset
	for (SoundInfo& info : infos) {
		header.size += sizeof(SoundEntry);
	}

	return header.size;
}

bool SectionSounds::fromFile(GMFile* gmf, Header &h, FILE* f, uint32_t offset) {
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

	// Read infos
	for (int i = 0; i < count; i++) {
		fseek(f, offsets[i], 0);

		SoundInfo info;
		fread(&info.entry, sizeof(SoundEntry), 1, f);

		infos.push_back(info);
	}

	return true;
}

bool SectionSounds::fromDir(GMFile* gmf, Header &h, string section_path) {
	header = h;

	vector<string> paths;

	// Enum file
	for (auto& fe : filesystem::directory_iterator(section_path)) {
		filesystem::path fp = fe.path(), fn = fp.filename();
		if (!filesystem::is_directory(fe) && fn.extension() == ".json") {
			paths.push_back(fp.string());
		}
	}
	count = paths.size();

	// Init infos
	for (int i = 0; i < count; i++) {
		SoundInfo info;
		infos.push_back(info);
	}

	// Load data
	for (const string& path : paths) {
		rapidjson::Document document;

		ifstream fin(path, ios::binary);
		if (!fin.is_open()) {
			cout << "File not found: " << path << endl;
			return false;
		}

		rapidjson::IStreamWrapper isw(fin);
		document.ParseStream(isw);

		// Load data
		int id = document["id"].GetInt();
		SoundInfo &info = infos[id];
		info.entry.groupID = document["groupID"].GetInt();
		info.entry.audioID = document["audioID"].GetInt();
		info.entry.volume = document["volume"].GetFloat();
		info.entry.pitch = document["pitch"].GetFloat();
		info.nameID = document["nameID"].GetInt();
		info.typeID = document["typeID"].GetInt();
		info.filenameID = document["filenameID"].GetInt();
		info.embedded = document["embedded"].GetBool();
		info.compressed = document["compressed"].GetBool();
	}

	return true;
}

bool SectionSounds::toFile(GMFile* gmf, FILE* f) {
	fseek(f, offset, 0);

	fwrite(&count, sizeof(uint32_t), 1, f);

	// Write offset
	uint32_t info_offset = offset + sizeof(uint32_t) + sizeof(uint32_t) * count;
	for (int i = 0; i < count; i++) {
		fwrite(&info_offset, sizeof(uint32_t), 1, f);
		info_offset += sizeof(SoundEntry);
	}

	return true;
}

bool SectionSounds::toDir(GMFile* gmf, string section_path) const {
	for (int i = 0; i < infos.size(); i++) {
		SoundInfo info = infos[i];
		string json_file = Util::join(section_path, info.name + ".json");

		FILE* fp = fopen(json_file.c_str(), "wb");

		char writeBuffer[65536];
		rapidjson::FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));

		rapidjson::Writer<rapidjson::FileWriteStream> writer(os);
		writer.StartObject();

		writer.Key("id");
		writer.Int(i);

		writer.Key("groupID");
		writer.Int(info.entry.groupID);

		writer.Key("audioID");
		writer.Int(info.entry.audioID);

		writer.Key("volume");
		writer.Double(info.entry.volume);

		writer.Key("pitch");
		writer.Double(info.entry.pitch);

		writer.Key("nameID");
		writer.Int(info.nameID);

		writer.Key("typeID");
		writer.Int(info.typeID);

		writer.Key("filenameID");
		writer.Int(info.filenameID);

		writer.Key("embedded");
		writer.Bool(info.embedded);

		writer.Key("compressed");
		writer.Bool(info.compressed);

		writer.EndObject();
		fclose(fp);
	}

	return true;
}

// Link strings and audio group
bool SectionSounds::linkFrom(GMFile* gmf) {
	SectionStrings* sectionStrings = (SectionStrings*) gmf->getSection(HEADER_STRINGS);
	SectionAudioGroups* sectionAudioGroups = (SectionAudioGroups*)gmf->getSection(HEADER_AUDIO_GROUPS);
	AudioGroupInfo group;

	for (SoundInfo& info : infos) {
		if (!sectionStrings->getStringByOffset(info.entry.nameOffset, info.nameID, info.name)) {
			cout << "No valid name found for sound " << info.entry.nameOffset << endl;
			return false;
		}

		if (!sectionStrings->getStringByOffset(info.entry.typeOffset, info.typeID, info.type)) {
			cout << "No valid type found for sound " << info.entry.typeOffset << endl;
			return false;
		}

		if (!sectionStrings->getStringByOffset(info.entry.filenameOffset, info.filenameID, info.filename)) {
			cout << "No valid filename found for sound " << info.entry.filenameOffset << endl;
			return false;
		}

		if (!sectionAudioGroups->getGroup(info.entry.groupID, group)) {
			cout << "No valid audio group found for sound " << info.entry.groupID << endl;
			return false;
		}
		info.group = group.name;
		group.name = "haha";

		info.embedded = (info.entry.flags & SOUND_EMBEDDED) != 0;
		info.compressed = (info.entry.flags & SOUND_COMPRESSED) != 0;
	}

	return true;
}

bool SectionSounds::linkTo(GMFile* gmf, FILE* f) {
	SectionStrings* sectionStrings = (SectionStrings*)gmf->getSection(HEADER_STRINGS);

	// Write sound info
	fseek(f, offset + sizeof(uint32_t) + sizeof(uint32_t) * count, 0);
	for (SoundInfo& info : infos) {
		info.entry.nameOffset = sectionStrings->getOffset(info.nameID);
		info.entry.typeOffset = sectionStrings->getOffset(info.typeID);
		info.entry.filenameOffset = sectionStrings->getOffset(info.filenameID);

		info.entry.flags = SOUND_NORMAL;
		if (info.embedded) info.entry.flags |= SOUND_EMBEDDED;
		if (info.compressed) info.entry.flags |= SOUND_COMPRESSED;

		info.entry._pad = 0;

		fwrite(&info.entry, sizeof(SoundEntry), 1, f);
	}
	return true;
}

bool SectionSounds::getSoundByAudioID(const int audioID, SoundInfo& sound) const {
	for (SoundInfo info : infos) {
		if (audioID == info.entry.audioID) {
			sound = info;
			return true;
		}
	}
	return false;
}
