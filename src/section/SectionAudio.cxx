#include "GMFile.h"
#include "utils.h"

uint32_t SectionAudio::calcSize(GMFile* gmf, uint32_t offset) {
	this->offset = offset;

	// Calc size
	uint32_t data_offset = offset + sizeof(uint32_t) + sizeof(uint32_t) * count;

	// Calc audio file offset
	for (uint32_t i = 0; i < count; i++) {
		if (i > 0) data_offset += Util::align(data_offset, 4); // skip first one
		offsets.push_back(data_offset); // record offset for toFile()

		data_offset += sizeof(uint32_t) + audio_files[i].length;
	}

	header.size = data_offset - offset;

	return header.size;
}

bool SectionAudio::fromFile(GMFile* gmf, Header &h, FILE* f, uint32_t offset) {
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

		AudioFile audio_file;
		fread(&audio_file.length, sizeof(uint32_t), 1, f);

		audio_file.data = new uint8_t[audio_file.length];
		fread(audio_file.data, sizeof(uint8_t), audio_file.length, f);

		audio_files.push_back(audio_file);
	}

	return true;
}

bool SectionAudio::fromDir(GMFile* gmf, Header &h, string section_path) {
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
		AudioFile audio_file;

		audio_file.filename = list[i].GetString();
		string audio_path = Util::join(section_path, audio_file.filename);

		FILE * fp = fopen(audio_path.c_str(), "rb");
		if (fp == NULL) {
			cout << "Failed to open: " << audio_path << endl;
			return false;
		}
		fseek(fp, 0L, SEEK_END);
		audio_file.length = ftell(fp);
		rewind(fp);

		audio_file.data = new uint8_t[audio_file.length];
		fread(audio_file.data, sizeof(uint8_t), audio_file.length, fp);
		fclose(fp);

		audio_files.push_back(audio_file);
	}

	fin.close();
	return true;
}

bool SectionAudio::toFile(GMFile* gmf, FILE* f) {
	fseek(f, offset, 0);

	fwrite(&count, sizeof(uint32_t), 1, f);

	// Write offset
	fseek(f, offset + sizeof(uint32_t), 0);
	for (int i = 0; i < count; i++) {
		fwrite(&offsets[i], sizeof(uint32_t), 1, f);
	}

	// Write audio data
	for (int i = 0; i < count; i++) {
		fseek(f, offsets[i], 0);
		fwrite(&audio_files[i].length, sizeof(uint32_t), 1, f);
		fwrite(audio_files[i].data, sizeof(uint8_t), audio_files[i].length, f);
	}

	return true;
}

bool SectionAudio::toDir(GMFile* gmf, string section_path) const {
	// Write json
	string json_file = Util::join(section_path, "data.json");

	FILE* fp = fopen(json_file.c_str(), "wb");

	char writeBuffer[65536];
	rapidjson::FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));

	rapidjson::Writer<rapidjson::FileWriteStream> writer(os);
	writer.StartArray();
	for (int i = 0; i < count; i++) {
		writer.String(audio_files[i].filename.c_str()); // assume audio id is sequential
	}
	writer.EndArray();

	fclose(fp);

	// Write data
	for (AudioFile audio_file : audio_files) {
		string audio_path = Util::join(section_path, audio_file.filename);

		FILE * fp = fopen(audio_path.c_str(), "wb");
		fwrite(audio_file.data, sizeof(uint8_t), audio_file.length, fp);
		fclose(fp);
	}

	return true;
}

bool SectionAudio::linkFrom(GMFile* gmf) {
	SectionSounds* sectionSounds = (SectionSounds*)gmf->getSection(HEADER_SOUNDS);

	for (int i = 0; i < count; i++) {
		SoundInfo sound;
		if (!sectionSounds->getSoundByAudioID(i, sound)) {
			cout << "No valid sound found for audio " << i << endl;
			return false;
		}
		audio_files[i].group = sound.group;
		audio_files[i].filename = sound.filename;
	}

	return true;
}

bool SectionAudio::linkTo(GMFile* gmf, FILE* f) {
	return true;
}
