
#include "GMFile.h"
#include "utils.h"

bool SectionGeneral::toJsonFile(string path) const {
	FILE* f = fopen(path.c_str(), "wb");

	char writeBuffer[65536];
	rapidjson::FileWriteStream os(f, writeBuffer, sizeof(writeBuffer));

	rapidjson::Writer<rapidjson::FileWriteStream> writer(os);
	writer.StartObject();

	writer.Key("debug");
	writer.Bool(gen8.debug);

	writer.Key("byteCodeVersion");
	writer.Uint(gen8.byteCodeVersion);

	writer.Key("filenameOffset");
	writer.Uint(gen8.filenameOffset);

	writer.Key("configOffset");
	writer.Uint(gen8.configOffset);

	writer.Key("lastObj");
	writer.Uint(gen8.lastObj);

	writer.Key("lastTile");
	writer.Uint(gen8.lastTile);

	writer.Key("gameID");
	writer.Uint(gen8.gameID);

	writer.Key("_pad1");
	writer.StartArray();
	for (uint32_t i : gen8._pad1) {
		writer.Uint(i);
	}
	writer.EndArray();

	writer.Key("nameOffset");
	writer.Uint(gen8.nameOffset);

	writer.Key("major");
	writer.Int(gen8.major);

	writer.Key("minor");
	writer.Int(gen8.minor);

	writer.Key("release");
	writer.Int(gen8.release);

	writer.Key("build");
	writer.Int(gen8.build);

	writer.Key("windowSizeX");
	writer.Int(gen8.windowSize.x);

	writer.Key("windowSizeY");
	writer.Int(gen8.windowSize.y);

	writer.Key("info");
	writer.Uint(gen8.info);

	writer.Key("md5");
	writer.StartArray();
	for (uint8_t c : gen8.md5) {
		writer.Uint(c);
	}
	writer.EndArray();

	writer.Key("crc32");
	writer.Uint(gen8.crc32);

	writer.Key("timestamp");
	writer.Uint64(gen8.timestamp);

	writer.Key("displayNameOffset");
	writer.Uint(gen8.displayNameOffset);

	writer.Key("activeTargets");
	writer.Uint(gen8.activeTargets);

	writer.Key("_unknown");
	writer.StartArray();
	for (uint32_t i : gen8._unknown) {
		writer.Uint(i);
	}
	writer.EndArray();

	writer.Key("appID");
	writer.Uint(gen8.appID);

	writer.Key("numberCount");
	writer.Uint(gen8.numberCount);

	writer.Key("numbers");
	writer.StartArray();
	for (int i = 0; i < gen8.numberCount; i++) {
		writer.Uint(numbers[i]);
	}
	writer.EndArray();

	writer.EndObject();

	fclose(f);
	return true;
}

uint32_t SectionGeneral::calcSize(GMFile* gmf, uint32_t offset) {
	// Calc size
	header.size = sizeof(GEN8) + sizeof(uint32_t) * gen8.numberCount;
	return header.size;
}

bool SectionGeneral::fromFile(GMFile* gmf, Header &h, FILE* f, uint32_t offset) {
	header = h;

	fseek(f, offset, 0);

	fread(&gen8, sizeof(GEN8), 1, f);
	numbers = new uint32_t[gen8.numberCount];
	fread(numbers, sizeof(uint32_t), gen8.numberCount, f);

	return true;
}

bool SectionGeneral::fromDir(GMFile* gmf, Header &h, string basepath) {
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
	gen8.debug = document["debug"].GetBool();
	gen8.byteCodeVersion = document["byteCodeVersion"].GetUint();
	gen8.filenameOffset = document["filenameOffset"].GetUint();
	gen8.configOffset = document["configOffset"].GetUint();
	gen8.lastObj = document["lastObj"].GetUint();
	gen8.lastTile = document["lastTile"].GetUint();
	gen8.gameID = document["gameID"].GetUint();

	auto _pad = document["_pad1"].GetArray();
	for (int i = 0; i < _pad.Size(); i++) {
		gen8._pad1[i] = _pad[i].GetUint();
	}

	gen8.nameOffset = document["nameOffset"].GetUint();
	gen8.major = document["major"].GetInt();
	gen8.minor = document["minor"].GetInt();
	gen8.release = document["release"].GetInt();
	gen8.build = document["build"].GetInt();
	gen8.windowSize.x = document["windowSizeX"].GetInt();
	gen8.windowSize.y = document["windowSizeY"].GetInt();
	gen8.info = (InfoFlag)document["info"].GetUint();

	auto md5 = document["md5"].GetArray();
	for (int i = 0; i < md5.Size(); i++) {
		gen8.md5[i] = md5[i].GetUint();
	}

	gen8.crc32 = document["crc32"].GetUint();
	gen8.timestamp = document["timestamp"].GetUint64();
	gen8.displayNameOffset = document["displayNameOffset"].GetUint();
	gen8.activeTargets = document["activeTargets"].GetUint();

	auto _unknown = document["_unknown"].GetArray();
	for (int i = 0; i < _unknown.Size(); i++) {
		gen8._unknown[i] = _unknown[i].GetUint();
	}

	gen8.appID = document["appID"].GetUint();
	gen8.numberCount = document["numberCount"].GetUint();

	numbers = new uint32_t[gen8.numberCount];
	auto _numbers = document["numbers"].GetArray();
	for (int i = 0; i < _numbers.Size(); i++) {
		numbers[i] = _numbers[i].GetUint();
	}

	return true;
}

bool SectionGeneral::toFile(GMFile* gmf, FILE* f, uint32_t offset) {
	fseek(f, offset, 0);

	fwrite(&gen8, sizeof(GEN8), 1, f);
	fwrite(numbers, sizeof(uint32_t), gen8.numberCount, f);

	return true;
}

bool SectionGeneral::toDir(GMFile* gmf, string section_path) const {
	string section_name = header.getName();

	string json_file = Util::join(section_path, "data.json");
	toJsonFile(json_file);

	return true;
}
